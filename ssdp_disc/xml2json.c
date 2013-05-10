#include <mxml.h>
#include "xml2json.h"

int node_is_text(mxml_node_t *node_txt) {
	int ret = 0;
	return ret;
}

char *xmlgettext(mxml_node_t **node_text, char *buffer, size_t len) {
	
	*buffer = 0;
	//	log("%s()\n", __FUNCTION__);
	for (; *node_text; *node_text = mxmlGetNextSibling(*node_text)) {
		const char *str = mxmlGetText(*node_text, 0);
		int len_buf = strlen(buffer);
		if (len_buf) {
			buffer[len_buf++] = ' ';
			buffer[len_buf] = 0;
		}
		strcpy(buffer + len_buf, str);
	}
	return buffer;
}

int isList(const char *name) {
	const char list[] = "List";
	if (!name) return 0;
	int len = strlen(name) - sizeof list + 1;

	if (len < 0) return 0;
	return !strcmp(list, name + len);
}
void jsonXmlGetAttributes(mxml_node_t *node, json_t *json_parent, xml_elem_t elm_type) {
	int i;
	mxml_attr_t *attr = node->value.element.attrs;
	
	for (i = 0; i < node->value.element.num_attrs; i++) {
		if (elm_type == ELM_OBJECT) {
			json_object_set_new(json_parent, attr->name, json_string(attr->value));
		} else {
			json_t *elm = json_object();
			json_object_set_new(elm, attr->name, json_string(attr->value));
				json_array_append_new(json_parent, elm);
			}
	}
		
}
json_t *xml2json_ex(mxml_node_t *node_parent, char *indent, xml_elem_t elm_type) {
	json_t *ret = 0;
	mxml_node_t *node;
	char indent2[1024];

	//	log("%s()\n", __FUNCTION__);
	sprintf(indent2, "  |%s", indent);

	for (node = node_parent; node != NULL; node = mxmlGetNextSibling(node)) {
		const char *elm_name = mxmlGetElement(node);
		const char *elm_value;
		json_t *json_child = 0;

		switch(node->type) {
		case MXML_TEXT:
			elm_value = mxmlGetText(node, 0);
			if (!strlen(elm_value)) break;
			char buffer[4096];
			log("%s-%s\n", indent2, xmlgettext(&node, buffer, sizeof(buffer)));
			ret = json_string(buffer);
			break;
		case MXML_ELEMENT:
			//			log("%s+%s array %d\n", indent2, elm_name, isList(elm_name));
			json_child = xml2json_ex(node->child, indent2, isList(elm_name) ? ELM_ARRAY : ELM_OBJECT);
			if (elm_type == ELM_OBJECT) {
				if (!ret) ret = json_object();
				json_object_set_new(ret, elm_name, json_child);
			} else if (elm_type == ELM_ARRAY) {
				if (!ret) ret = json_array();
				json_array_append_new(ret, json_child);
			} 
			jsonXmlGetAttributes(node, ret, elm_type);
			break;
		default:
			log("Unattended type : %d", node->type);
			break;
		}
	}
	return ret;
}

json_t *xml2json(const char *content) {
	json_t *json_root = 0;
	mxml_node_t *tree = mxmlLoadString(NULL, content, MXML_TEXT_CALLBACK);
	//	mxml_node_t *mxml_dev = mxmlFindElement(tree, tree,	"device", NULL, NULL,	MXML_DESCEND);
	fprintf( stderr, "%s(%zu)\n", __FUNCTION__, strlen(content));
	if (tree) {
		json_root = xml2json_ex(tree,  "  ", ELM_OBJECT);
	}
	if (tree)  mxmlDelete(tree);
	return json_root;
}

json_t *xmlFile2json(const char *file) {
	json_t *json_root = 0;
	FILE *fp = fopen(file, "r");
	mxml_node_t *tree = mxmlLoadFile(NULL, fp, MXML_TEXT_CALLBACK);
	//	mxml_node_t *mxml_dev = mxmlFindElement(tree, tree,	"device", NULL, NULL,	MXML_DESCEND);
	//	fprintf( stderr, "%s(%d)\n", __FUNCTION__, strlen(content));
	if (tree) {
		json_root = xml2json_ex(tree, "  ", ELM_OBJECT);
	}
	if (tree)  mxmlDelete(tree);
	json_dumpf(json_root, stdout, JSON_INDENT(2));

	return json_root;
}
