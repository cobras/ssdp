#ifndef __XML2JSON_H__
#define __XML2JSON_H__
#include <jansson.h>
#include "common.h"
typedef enum {
	ELM_TXT = 0,
	ELM_OBJECT,
	ELM_ARRAY
} xml_elem_t;

json_t *xml2json_ex(mxml_node_t *node_parent, char *indent, xml_elem_t elm_type);
json_t *xml2json(const char *content);
json_t *xmlFile2json(const char *file);



#endif
