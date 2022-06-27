//Imports
#include "dr_api.h"
#include "dr_events.h"
#include "drmgr.h"
#include "drsyms.h"
#include "drwrap.h"
#include <stdint.h>

/**
 * @brief Target function pre-execution handler
 * 
 * @param ctx DynamoRIO context
 * @param data User data
 */
static void onBeforeTarget(void *ctx, void **data)
{
  //Get the arguments
  int a = (intptr_t)drwrap_get_arg(ctx, 0);
  int b = (intptr_t)drwrap_get_arg(ctx, 1);

  //Skip
  if (true)
  {
    //Compute the sum
    int sum = a + b;

    //Alter the sum
    int altered = sum - 100;

    //Print
    dr_printf("[Proxy] %i + %i = %i\n", a, b, sum);
    dr_printf("[Proxy] Returning %i instead\n", altered);

    //Skip
    drwrap_skip_call(ctx, (void *)&altered, sizeof(altered));
  }
}

/**
 * @brief DynamoRIO module loaded handler
 * 
 * @param ctx DynamoRIO context
 * @param module Module that was loaded
 * @param loaded Whether or not the module was loaded?
 */
static void onModuleLoad(void *ctx, const module_data_t *module, bool loaded)
{
  //Get the target function offset
  size_t offset;
  drsym_error_t error = drsym_lookup_symbol(module->full_path, "add(int, int)", &offset, DRSYM_DEMANGLE);

  //Get the target function address
  app_pc address = module->start + offset;

  if (error == DRSYM_SUCCESS)
  {
    //Wrap the target function
    drwrap_wrap(address, onBeforeTarget, NULL);
  }
}

/**
 * @brief DynamoRIO exit handler
 */
static void onExit()
{
  //Cleanup
  drsym_exit();
  drwrap_exit();
  drmgr_exit();
}

/**
 * @brief DynamoRIO entry point
 * 
 * @param id Client ID
 * @param argc Argument count
 * @param argv Argument values
 */
DR_EXPORT void dr_client_main(client_id_t id, int argc, const char *argv[])
{
  //Update client metadata
  dr_set_client_name("Proxy", "https://github.com/Wakeful-Cloud/cpp-proxy/issues");

  //Initialize DynamoRIO
  drmgr_init();
  drwrap_init();
  drsym_init(0);

  //Register handlers
  dr_register_exit_event(onExit);
  drmgr_register_module_load_event(onModuleLoad);

  //Configure function wrapping (Improve performance)
  drwrap_set_global_flags(DRWRAP_NO_FRILLS | DRWRAP_FAST_CLEANCALLS);
}