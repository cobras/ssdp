#ifndef __JSON2SOAP_H__
#define __JSON2SOAP_H__

struct s_json_action_args {
	char name[256];
	char value[256];
};


struct s_json_action {
	char *host;
  char *port;
	char *url;
	char *service_type;
	char *action;
  struct  s_json_action_args *action_args;
};

json_t *json_exec_action(const struct s_json_action *);

#endif


