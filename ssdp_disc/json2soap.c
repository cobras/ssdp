#include "common.h"
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mxml.h>
#include "xml2json.h"
#include "json2soap.h"


int get_fd(const char *ip, const char *port) {
	int fd = socket(AF_INET, SOCK_STREAM, 0);

	if (fd > 0) {
		struct sockaddr_in addr = {
			.sin_port = htons(atoi(port)),
			.sin_family = AF_INET,
		};
		//		socklen_t len_addr = sizeof(addr);
		if (inet_pton(AF_INET, ip, &addr.sin_addr.s_addr) != 1)
			log("Error :%s", strerror(errno));
		
		if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) >= 0) {
			/* char http_msg[1024]; */
			
			/* snprintf(http_msg, sizeof(http_msg), "GET %s HTTP/1.0\r\n\r\n", url); */
			/* size_t nb_bytes = write(fd, http_msg, strlen(http_msg)); */
		} else {
			log("Error :%s", strerror(errno));
			fd = -1;
		}
	}	
	return fd;
}
int wait_fd(int fd) {
	int ret = -1;
	fd_set rdfs;
	FD_ZERO(&rdfs);
	FD_SET(fd, &rdfs);
	ret = select(fd + 1, &rdfs, 0, 0, 0);
	if (FD_ISSET(fd, &rdfs)) {
		ret = 0;
	}
	return ret;
}
char *read_response(int fd) {
	char *buffer = 0;
	int sizeof_buf = 1;
	int len_stream = 0;
	int nb_bytes;
	do  {	
		if (len_stream + BLOC_LEN + 1 > sizeof_buf) {
			sizeof_buf += BLOC_LEN; 
			buffer = realloc(buffer, sizeof_buf);
			log("realloc(%p, %d)\n", buffer, sizeof_buf - len_stream);
		}
		
		nb_bytes = read(fd, buffer + len_stream, sizeof_buf - len_stream);
		if (nb_bytes < 0) break;
		len_stream += nb_bytes;
	}
	while (nb_bytes > 0);	
	buffer[len_stream] = 0;
	return buffer;
}
json_t *json_exec_action(const struct s_json_action *json_action) {
	int fd = get_fd(json_action->host, json_action->port);
	char http_hdr[4096];
	char http_msg[4096];
	json_t *ret = 0;
	const char hdr_tpl[] = 
		"POST %s HTTP/1.1\n"\
		"HOST: %s:%s\n"\
		"Content-Type: text/xml; charset=utf-8\n"\
		"User-Agent: UPnP/1.0 DLNADOC/1.50\n"\
		"Connexion:Close\n"\
		"SOAPACTION: \"%s#%s\"\n"\
		"Content-Length: %d\n\n";

	const char msg_tpl[] = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"\
		"<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\""\
		" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"\
		"<s:Body><u:%s xmlns:u=\"%s\">"\
		"%s</u:%s></s:Body></s:Envelope>\n";
		;
	char *args = 0;
	int i;
	char args_buf[1024];
	int len = 0;

	for ( i = 0; *json_action->action_args[i].name; i++) {
		snprintf(args_buf, sizeof(args_buf) ,
						 "<%s>%s</%s>", 
						 json_action->action_args[i].name, 
						 json_action->action_args[i].value, 
						 json_action->action_args[i].name);
		int len_cur = strlen(args_buf);
		args = realloc(args, len_cur + 1 + ((len) ? strlen(args) : 0));
		memcpy(args + len, args_buf, strlen(args_buf) + 1);
		len += strlen(args_buf);
	}
	if (fd > 0) {
		int len;

		snprintf(http_msg, sizeof(http_msg), msg_tpl,
						 json_action->action, 
						 json_action->service_type,
						 (args) ? args : "",
						 json_action->action);
		if (args) free(args);
		len = strlen(http_msg);

		snprintf(http_hdr, sizeof(http_hdr), hdr_tpl, 
						 json_action->url,
						 json_action->host, json_action->port,
						 json_action->service_type, json_action->action,
						 len);

		size_t nb_bytes = write(fd, http_hdr, strlen(http_hdr));
		nb_bytes = write(fd, http_msg, strlen(http_msg));
		log("sending data %zu\n", nb_bytes);
		printf("ret: \n%s", http_msg);
	}
	//	if (wait_fd(fd) > 0) {
	char *buffer = read_response(fd);

	log("retour : \n%s", buffer);
	char * content = strstr(buffer, "\r\n\r\n");
	if (!content) content = strstr(buffer, "\n\n");
	if (!content) goto err_fmt;
	while (*content == '\r' || *content == '\n') content++;
	ret = xml2json(content);
	if (buffer) free(buffer);
	//	}
	return ret;
 err_fmt:
	log("Error: malformed http body\n");
	if (buffer) free(buffer);
	return ret;
}
#ifdef TEST_SOAP

int main(int argc, char **argv) {
	/* struct s_args args[] = { */
	/* 	{.name=""} */
	/* }; */

	/* json_t *json_object = json_exec_action( */
	/* 								 "192.168.1.2", */
	/* 								 "49152", */
	/* 								 "/upnp/control/cds", */
	/* 								 "urn:schemas-upnp-org:service:ContentDirectory:1", */
	/* 								 "GetSearchCapabilities", */
	/* 								 args */
	/* 								 ); */

	/* json_t *json_object = json_exec_action( */
	/* 								 "192.168.1.2", */
	/* 								 "49152", */
	/* 								 "/upnp/control/cds", */
	/* 								 "urn:schemas-upnp-org:service:ContentDirectory:1", */
	/* 								 "GetSystemUpdateID", */
	/* 								 args */
	/* 								 ); */

	struct s_json_action_args args[] = {
		{.name="ObjectID", .value="0"},
		{.name="BrowseFlag", .value="BrowseDirectChildren"},
		{.name="Filter", .value="*"},
		{.name="StartingIndex", .value="0"},
		{.name="RequestedCount", .value="100"},
		{.name="SortCriteria", .value=""},
		{.name=""}
	};
	struct s_json_action json_action = {
		.host = "192.168.1.2",
		.port = "49152",
		.url = "/upnp/control/cds",
		.service_type = "urn:schemas-upnp-org:service:ContentDirectory:1",
		.action = "Browse",
		.action_args = args,
 /* = { */
 /* 			{.name="ObjectID", .value="0"}, */
 /* 			{.name=""}, */
 /* 		} */
	};

	json_t *json_object = json_exec_action(&json_action);
	
	log("Dumping data : \n");
	json_dumpf(json_object, stdout, JSON_INDENT(2));
	json_decref(json_object);
	return 0;
}
#endif
/* #if 0 */

/* 	json_exec_action( */
/* 									 "192.168.1.1", */
/* 									 "9000", */
/* 									 "ContentDirectory/Control", */
/* 									 "urn:schemas-upnp-org:service:ContentDirectory:1", */
/* 									 "GetSearchCapabilities" */
/* 									 ); */
/* #endif */

/* 	return 0; */
/* } */

