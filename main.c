#include "k2sh.h"

int main() {
    shell_state_t shell_state;
    
    shell_init(&shell_state);
    
    shell_mainloop(&shell_state);
    
    shell_cleanup(&shell_state);
    
    return shell_state.last_exit_status;
}