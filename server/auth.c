// PAM authentication support
#include <security/pam_appl.h>
#include <stdlib.h>
#include <string.h>

#include "logger.h"

extern struct nfs_logger* logger;

static int func_conv(int num_msg, const struct pam_message **msg, struct pam_response **resp, void *appdata_ptr)
{
    struct pam_response *pam_reply;

    pam_reply = calloc(1, sizeof(struct pam_response));
    if(pam_reply == NULL)
        return PAM_BUF_ERR;
    pam_reply->resp = strdup(appdata_ptr);
    pam_reply->resp_retcode = 0;

    *resp = pam_reply;
    return PAM_SUCCESS;
}

int are_credentials_ok(char* username, char* password) {
    int ret = 0;
    const struct pam_conv local_conversation = { func_conv, password };
    
    pam_handle_t *local_auth_handle = NULL;
    int retval = pam_start("system-auth", username, &local_conversation, &local_auth_handle);
    if(retval != PAM_SUCCESS) {
        nfs_log_error(logger, "Failed to start PAM transaction - error: %d", retval);
        goto err;
    }

    retval = pam_authenticate(local_auth_handle, 0);
    if(retval != PAM_SUCCESS) {
        if(retval == PAM_AUTH_ERR || retval == PAM_USER_UNKNOWN || retval == PAM_PERM_DENIED) {
            nfs_log_error(logger, "Failed to authenticate user `%s` - invalid username/password", username);
            goto err_pam_end;
        } else {
            nfs_log_error(logger, "Failed to authenticate user `%s` - PAM error (%d)", username, retval);
            goto err_pam_end;
        }
    }

    // retval == PAM_SUCCESS - username/password are correct
    ret = 1;

err_pam_end:
    retval = pam_end(local_auth_handle, retval);
    if(retval != PAM_SUCCESS)
        nfs_log_error(logger, "Failed to end PAM transaction - PAM error (%d)", retval);
err:
    return ret;
}
