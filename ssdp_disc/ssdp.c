/* Receiver/client multicast Datagram example. */

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <sys/types.h>
#include <ifaddrs.h>
#include <jansson.h>
#include <ctype.h>
#include <mxml.h>
#include "xml2json.h"
#include "common.h"

#include "ssdp.h"
int read_ssp_resp(int fd, char **http_hdr, char **http_payload) {
	size_t len_stream = 0;
	int is_hdr = 1;
	size_t sizeof_buf = 1;
	char *buffer = 0;
	ssize_t nb_bytes = 0;
	do  {
		if (len_stream + BLOC_LEN + 1 > sizeof_buf) {
			sizeof_buf += BLOC_LEN; 
			buffer = realloc(buffer, sizeof_buf);
			log("realloc(%p, %zd)\n", buffer, sizeof_buf - len_stream);
		}

		nb_bytes = read(fd, buffer + len_stream, sizeof_buf - len_stream);
		log("debug: read(%d,%p,%zd) = %zu \n", fd, 
				buffer + len_stream, sizeof_buf - len_stream, nb_bytes);
		if (nb_bytes > 0) {
			/* char *cur = buffer + len_stream; */
			len_stream += nb_bytes;
			buffer[len_stream] = 0;
			if (is_hdr) {
				char *end_hdr = strstr(buffer, "\n\n");
				if (!end_hdr) end_hdr = strstr(buffer, "\r\n\r\n");
				
				if (end_hdr) {
					*end_hdr = 0;
					*http_hdr = buffer;
					is_hdr = 0;
					while(*end_hdr == '\n' || *end_hdr == '\r') end_hdr++;
					buffer = strdup(end_hdr);
					len_stream = strlen(end_hdr);
					sizeof_buf = len_stream;
				} 
			}
		}
	} while (nb_bytes > 0);
	*http_payload = buffer;
	return 0;
}

int get_ssdp_content(const char *url, const char *ip, const char *port, json_t *json_root) {
	log("%s('%s',%s:%s)\n", __FUNCTION__, url,  ip, port);
	char *http_hdr = 0;
	char *http_payload = 0;

	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd > 0) {
		struct sockaddr_in addr = {
			.sin_port = htons(atoi(port)),
			.sin_family = AF_INET,
		};
		if (inet_pton(AF_INET, ip, &addr.sin_addr.s_addr) != 1)
			log("Error :%s", strerror(errno));

		if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) >= 0) {
			char http_msg[1024];
			
			snprintf(http_msg, sizeof(http_msg), "GET %s HTTP/1.0\r\n\r\n", url);
			size_t nb_bytes = write(fd, http_msg, strlen(http_msg));
			log("write(%d, %s, %zu) = %zu\n", fd, http_msg, strlen(http_msg), nb_bytes);

			if (nb_bytes == strlen(http_msg)) {
				int status = 0;
				const char status_tpl[] = "HTTP/1.0 ";
				read_ssp_resp(fd, &http_hdr, &http_payload);
				printf("status = %d \n", status);
				if (http_hdr) {
					char *cur;
					for (cur = strtok(http_hdr, "\n"); cur; cur = strtok(0, "\n")) {
						if (!strncmp(cur, status_tpl, strlen(status_tpl))) {
							char *status_str = cur + strlen(status_tpl);
							status = atoi(status_str);
							log("Status : %d\n", status);
						} else {
							char *name = cur;
							char *value = strchr(name, ':');
							if (value) {
								*value = 0;
								value += 2;							
								char *end_value = value + strlen(value) - 1;
								while (isspace(*end_value)) *end_value-- = 0;
								
								log("hdr('%s','%s')\n", name, value);
							} else log("ukn: %s", cur);
						}
					}
				}
				if (http_payload) {
					json_t *js_elm = xml2json(http_payload);
					if (js_elm) json_object_set_new(json_root, "content", js_elm);
					//log("Content : %s\n", buffer + start_content_pos);
					//					log("from %s\n", ip);
					json_dumpf(json_root, stdout, JSON_INDENT(2));
					/* if (!strcmp(ip, "192.168.1.2")) exit (1); */
				}else {
					log("return error %d\n", status);
				}

				if (http_hdr) free(http_hdr);
			} 
		}	 else log("connect error: %s", strerror(errno));
	} else log("socket: %s", strerror(errno));
	return 0;
}

int parse_location(const char *location, json_t **js_obj) {
	char *url = strdup(location);

	log("Url : %s\n", url);
	const char url_start_str[] = "http://";
	char *cur = url;
	if ((cur = strstr(url, url_start_str))) {
		char *ip = cur + strlen(url_start_str);
		if (ip) {
			char *port = strchr(ip, ':');
			if (port) {
				*port = 0;
				port++;
				char *end_url = strchr(port, '/');
				if (end_url) {
					*end_url = 0;
					*js_obj = json_object();
					get_ssdp_content(location, ip, port, *js_obj);
				} else {
					log("error: can't find end_url");
				}
			}
		}
		
	}

	return 0;
}

json_t *parse_multicast_payload(char *payload)  {
	char *line = payload;
  json_t *elm = json_object();

	for(line = strtok(payload, "\r\n"); line; line = strtok(0, "\r\n")) {
		if (strstr(line, ":")) {
			char *prop = line;
			char *value = strchr(prop, ':');

			if (!value) {
				log("Parse Error on %s\n", payload);
				continue;
			}
			*value = 0;
			value++;
			TRIM_R(value);
			TRIM_R(prop);
			json_object_set_new(elm, prop, json_string(value));
			if (!strcmp(prop, "LOCATION")) {
				log("parsing : %s\n", value);
				parse_location(value, &elm); 
				break;
			}
			
		}		
	}
	json_dumpf(elm, stdout, JSON_INDENT(2));
	return elm;
}

struct sockaddr *get_ip(char *ifname) {
	struct ifaddrs *ifaddr, *ifa;

	if (getifaddrs(&ifaddr) == -1) {
		perror("getifaddrs");
		exit(EXIT_FAILURE);
	}

	/* Walk through linked list, maintaining head pointer so we
		 can free list later */

	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifname && ifa->ifa_addr == NULL) continue;
		if (strcmp(ifname, ifa->ifa_name)) continue;
		if (ifa->ifa_addr->sa_family != AF_INET) continue;
		//		if (ifa->ifa_addr & ifa->ifa_netmask)
		log("ifname found %s\n", ifa->ifa_name);
		log ("\tAddress: %s\n", inet_ntoa (((struct sockaddr_in *) ifa->ifa_addr)->sin_addr));
		return ifa->ifa_addr;
	}
	free(ifaddr);
	return 0;
}

int get_ssdp_socket(char *ifname) {
	struct sockaddr_in localSock;
	struct ip_mreq group;
	int sd;

	sd = socket(AF_INET, SOCK_DGRAM, 0);

	if(sd < 0)
		{
			perror("Opening datagram socket error");
			return -1;
		}
	else
		log("Opening datagram socket....OK.\n");
 
	int reuse = 1;
	if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0)
		{
			perror("Setting SO_REUSEADDR error");
			close(sd);
			return -1;
		}
	else
		log("Setting SO_REUSEADDR...OK.\n");
 
	/* Bind to the proper port number with the IP address */
	/* specified as INADDR_ANY. */
	memset((char *) &localSock, 0, sizeof(localSock));
	localSock.sin_family = AF_INET;
	localSock.sin_port = htons(1900);
	localSock.sin_addr.s_addr = INADDR_ANY;
	if(bind(sd, (struct sockaddr*)&localSock, sizeof(localSock)))
		{
			perror("Binding datagram socket error");
			close(sd);
			return -1;
		}
	else
		log("Binding datagram socket...OK.\n");
 
	/* Join the multicast group 226.1.1.1 on the local 203.106.93.94 */
	/* interface. Note that this IP_ADD_MEMBERSHIP option must be */
	/* called for each local interface over which the multicast */
	/* datagrams are to be received. */
	group.imr_multiaddr.s_addr = inet_addr("239.255.255.250");
	struct sockaddr_in *sock = (struct sockaddr_in *) get_ip(ifname);
	group.imr_interface.s_addr = sock->sin_addr.s_addr;
	if(setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group, sizeof(group)) < 0)
		{
			perror("Adding multicast group error");
			close(sd);
			return -1;
		}
	else
		log("Adding multicast group...OK.\n");
 
	return sd;
}
int ssdp_success_dump(json_t *jsobj, void *cb_payload) {
	printf("Object found !!! \n");
	const char *udn;
	const char *xmlns;
	
	/* int ret =  json_unpack(jsobj, "{:{s:s}}", "device", */
	/* 											 ); */
	/* printf("UDN=%s\n\n", udn); */

	json_dumpf(jsobj, stdout, JSON_INDENT(2));
	return 0;
}

int ssdp_failed_dump(const char * err) {
	printf(err);
	return 0;
}

int discover_ssdp(char *ifname, 
									ssdp_success_t *on_success, ssdp_failed_t *on_failed, void *cb_payload) {
	int sd = get_ssdp_socket(ifname);
	char payload[4096];
	int datalen = sizeof(payload);

	if (!on_success) on_success = ssdp_success_dump;
	if (!on_failed) on_failed = ssdp_failed_dump;
	if (sd < 0) return -1;

	while (1) {
		fd_set readfs;
		struct timeval tv = {
			.tv_sec = 5,
			.tv_usec = 0,
		};
		
		FD_ZERO(&readfs);
		FD_SET(sd, &readfs);
		int max_fd = sd;
		int ret_sel = select(max_fd + 1, &readfs, 0, 0, &tv);

		if (ret_sel < 0) {
			perror("select");
			exit(0);
		}
		
		if (FD_ISSET(sd, &readfs)) {
			ssize_t nb_bytes = read(sd, payload, datalen);
			if (nb_bytes < 0) {
				perror("read error");
				on_failed("read error");
				return -1;
			}
			log("read(%d,data, %d) = %zd\n", sd, datalen, nb_bytes);
			//			log("The message from multicast server is: \"%s\"\n", payload);
			log("Parsing ...\n");
			payload[nb_bytes] = 0;
			json_t *data = parse_multicast_payload(payload);
			if (data) {
				on_success(data, cb_payload);
				json_decref(data);
			}
			
		}
	}

	return 0;
}

int main(int argc, char **argv) {

	if (argc < 2) {
		fprintf(stderr, "Usage: %s <ifname>\n", argv[0]);
		return -1;
	}

	discover_ssdp(argv[1], 0, 0, 0);
#if 0
	json_t *res;
	const char *url[] = {
		"http://192.168.1.2:49152/description.xml",
		"http://192.168.1.1:1900/wfc.xml",
	};
	parse_location(url[1], &res);
	json_dumpf(res, stdout, JSON_INDENT(2));
	return 0;
#endif

	/* xmlFile2json("igd.xml"); */
	/* return 0; */
	/* Create a datagram socket on which to receive. */
	return 0;
}

