//Imports
#include "dr_api.h"
#include "dr_events.h"
#include "drmgr.h"
#include "drwrap.h"
#include <stdint.h>
// #include <cstdint>

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
  //Get the target function address
  app_pc targetAddress = (app_pc)dr_get_proc_address(module->handle, "_Z3addii");

  if (targetAddress != NULL)
  {
    //Wrap the target function
    drwrap_wrap(targetAddress, onBeforeTarget, NULL);
  }
}

/**
 * @brief DynamoRIO exit handler
 */
static void onExit()
{
  //Cleanup
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

  //Register handlers
  dr_register_exit_event(onExit);
  drmgr_register_module_load_event(onModuleLoad);
}