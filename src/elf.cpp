//Imports
#include "elf.hpp"
#include <cxxabi.h>
#include <fcntl.h>
#include <gelf.h>
#include <iostream>
#include <stdexcept>
#include <string.h>
#include <string>
#include <unistd.h>
#include <vector>

std::vector<Symbol> getSymbols(std::string path)
{
  //Set the ELF version
  elf_version(EV_CURRENT);

  //Open the file
  errno = 0;
  int fd = open(path.c_str(), O_RDONLY);

  //Error
  if (fd < 0)
  {
    //Get the actual error
    int error = errno;

    throw std::runtime_error("[ELF] Failed to open executable! Error: " + std::string(strerror(error)));
  }

  //Convert to ELF handle
  Elf *elf = elf_begin(fd, ELF_C_READ, NULL);

  //Error
  if (elf == NULL)
  {
    //Get the actual error
    int error = elf_errno();

    throw std::runtime_error("[ELF] Failed to convert to ELF handle! Error: " + std::string(strerror(error)));
  }

  //Ensure the ELF is an executable
  if (elf_kind(elf) != ELF_K_ELF)
  {
    throw std::runtime_error("[ELF] File is not an executable type ELF!");
  }

  //Get the ELF symbol table and aggregate symbols
  std::vector<Symbol> symbols;
  Elf_Scn *elfSection = NULL;
  while ((elfSection = elf_nextscn(elf, elfSection)) != NULL)
  {
    //Get the section header
    GElf_Shdr elfSectionHeader;
    gelf_getshdr(elfSection, &elfSectionHeader);

    if (elfSectionHeader.sh_type == SHT_SYMTAB)
    {
      //Get the data
      Elf_Data *elfData = elf_getdata(elfSection, NULL);

      //Iterate over ELF symbols
      for (std::size_t i = 0; i < (elfSectionHeader.sh_size / elfSectionHeader.sh_entsize); ++i)
      {
        //Get the ELF symbol
        GElf_Sym elfSymbol;
        gelf_getsym(elfData, i, &elfSymbol);

        //Get the mangled name
        const char* mangled = elf_strptr(elf, elfSectionHeader.sh_link, elfSymbol.st_name);

        //Skip empty symbols
        if (mangled == NULL || strlen(mangled) == 0)
        {
          continue;
        }

        //Get the symbol name
        std::string unmangled;

        //Detect if demangling is required
        if (strlen(mangled) >= 2 && mangled[0] == '_' && mangled[1] == 'Z')
        {
          //Demangle
          int status;
          char* demangled = abi::__cxa_demangle(mangled, NULL, NULL, &status);

          if (status == 0)
          {
            //Update the symbol name
            unmangled = std::string(demangled);
          }
          else
          {
            //Update the symbol name
            unmangled = std::string(mangled);
          }

          //Free the demangled name
          free(demangled);
        }
        else
        {
          //Update the symbol name
          unmangled = std::string(mangled);
        }

        //Translate the GELF symbol
        Symbol symbol;
        symbol.address = elfSymbol.st_value;
        symbol.mangledName = mangled;
        symbol.unmangledName = unmangled;

        //Add the symbol
        symbols.push_back(symbol);
      }
    }
  }

  //Cleanup the elf
  elf_end(elf);

  //Close the file
  close(fd);

  return symbols;
}