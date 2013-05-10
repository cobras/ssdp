#ifndef __SSDP_H__
#define __SSDP_H__



typedef int (ssdp_success_t)(json_t *obj, void *cb_payload);
typedef int (ssdp_failed_t)(const char *err);

int discover_ssdp(char *ifname, ssdp_success_t *on_success, ssdp_failed_t *on_failed, 
									void *cb_payload);
int get_ssdp_socket(char *ifname);

#endif
