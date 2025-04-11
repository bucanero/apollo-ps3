#include <sysutil/sysutil.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <string.h>

#include "menu.h"
#include "saves.h"
#include "common.h"


static char* get_xml_account_id(xmlNode * a_node)
{
	xmlNode *cur_node = NULL;
	char *value;

	for (cur_node = a_node->children; cur_node; cur_node = cur_node->next)
	{
		if (cur_node->type != XML_ELEMENT_NODE)
			continue;

		if (xmlStrcasecmp(cur_node->name, BAD_CAST "user") == 0)
		{
			value = (char*) xmlGetProp(cur_node, BAD_CAST "account_id");
			if (value)
			{
				LOG("xml account_id=%s", value);
                return value;
			}
		}
	}

    return NULL;
}

static xmlNode* _get_owner_node(xmlNode *a_node, xmlChar *owner_name)
{
    xmlNode *cur_node = NULL;

    for (cur_node = a_node; cur_node; cur_node = cur_node->next)
    {
        if (cur_node->type != XML_ELEMENT_NODE)
            continue;

        if ((xmlStrcasecmp(cur_node->name, BAD_CAST "owner") == 0) && (xmlStrcmp(xmlGetProp(cur_node, BAD_CAST "name"), owner_name) == 0))
            return cur_node;
    }

    return NULL;
}

/**
 * Parse a file called "owners.xml", 
 * walk down the DOM, and add the name/account-id of the 
 * xml elements nodes.
 */
void add_xml_owners(const char *xmlfile, list_t* usr_list)
{
    xmlDoc *doc = NULL;
    xmlNode *root_element = NULL;
    xmlNode *cur_node = NULL;
    char *value;
    option_value_t* optval;

    /*parse the file and get the DOM */
    doc = xmlParseFile(xmlfile);

    if (doc == NULL)
    {
        LOG("XML: could not parse file %s", xmlfile);
        return;
    }

    /*Get the root element node */
    root_element = xmlDocGetRootElement(doc);

    for (cur_node = root_element->children; cur_node; cur_node = cur_node->next)
    {
        if (cur_node->type != XML_ELEMENT_NODE)
            continue;

        if (xmlStrcasecmp(cur_node->name, BAD_CAST "owner") == 0)
        {
            value = (char*) xmlGetProp(cur_node, BAD_CAST "name");
            if (!value)
                continue;

            LOG("Adding Owner=%s", value);
            char* aid = get_xml_account_id(cur_node);
            if (!aid)
                continue;

            optval = malloc(sizeof(option_value_t));
            optval->name = strdup(value);
            optval->value = strdup(aid);
            list_append(usr_list, optval);
        }
    }

    /*free the document */
    xmlFreeDoc(doc);

    /*
     *Free the global variables that may
     *have been allocated by the parser.
     */
    xmlCleanupParser();
    return;
}

int save_xml_owner(const char *xmlfile)
{
    xmlDocPtr doc = NULL;       /* document pointer */
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;/* node pointers */
    char buff[SYSUTIL_SYSTEMPARAM_CURRENT_USERNAME_SIZE+1];

    snprintf(buff, sizeof(buff), "User %08d", apollo_config.user_id);
    sysUtilGetSystemParamString(SYSUTIL_SYSTEMPARAM_ID_CURRENT_USERNAME, buff, SYSUTIL_SYSTEMPARAM_CURRENT_USERNAME_SIZE);

    /*parse the file and get the DOM */
    doc = xmlReadFile(xmlfile, NULL, XML_PARSE_NONET | XML_PARSE_NOBLANKS);

    if (doc)
    {
        /*Get the root element node */
        root_node = xmlDocGetRootElement(doc);
        node = _get_owner_node(root_node->children, BAD_CAST buff);

        if (node)
        {
            xmlUnlinkNode(node);
            xmlFreeNode(node);
        }
    }
    else
    {
        LOG("XML: could not parse data, creating new file '%s'", xmlfile);

        /* 
        * Creates a new document, a node and set it as a root node
        */
        doc = xmlNewDoc(BAD_CAST "1.0");
        root_node = xmlNewNode(NULL, BAD_CAST "apollo");
        xmlNewProp(root_node, BAD_CAST "version", BAD_CAST APOLLO_VERSION);
        xmlDocSetRootElement(doc, root_node);
    }

    /* 
     * xmlNewChild() creates a new node, which is "attached" as child node
     * of root_node node. 
     */
    node = xmlNewChild(root_node, NULL, BAD_CAST "owner", NULL);
    xmlNewProp(node, BAD_CAST "name", BAD_CAST buff);

	node1 = xmlNewChild(node, NULL, BAD_CAST "console", NULL);

    sprintf(buff, "%016lX %016lX", apollo_config.idps[0], apollo_config.idps[1]);
    xmlNewProp(node1, BAD_CAST "idps", BAD_CAST buff);

    sprintf(buff, "%016lX %016lX", apollo_config.psid[0], apollo_config.psid[1]);
    xmlNewProp(node1, BAD_CAST "psid", BAD_CAST buff);

	node1 = xmlNewChild(node, NULL, BAD_CAST "user", NULL);

    sprintf(buff, "%08d", apollo_config.user_id);
    xmlNewProp(node1, BAD_CAST "id", BAD_CAST buff);

    sprintf(buff, "%016lx", apollo_config.account_id);
    xmlNewProp(node1, BAD_CAST "account_id", BAD_CAST buff);

    /* 
     * Dumping document to file
     */
    xmlSaveFormatFileEnc(xmlfile, doc, "UTF-8", 1);

    /*free the document */
    xmlFreeDoc(doc);

    /*
     *Free the global variables that may
     *have been allocated by the parser.
     */
    xmlCleanupParser();
    file_chmod(xmlfile);

    return(0);
}

char* get_xml_title_name(const char *xmlfile)
{
    xmlDoc *doc = NULL;
    xmlNode *root_element = NULL;
    xmlNode *cur_node = NULL;
    char *ret = NULL;

    /*parse the file and get the DOM */
    doc = xmlParseFile(xmlfile);
    if (!doc)
    {
        LOG("XML: could not parse file %s", xmlfile);
        return NULL;
    }

    /*Get the root element node */
    root_element = xmlDocGetRootElement(doc);

    for (cur_node = root_element->children; cur_node; cur_node = cur_node->next)
    {
        if (cur_node->type != XML_ELEMENT_NODE)
            continue;

        if (xmlStrcasecmp(cur_node->name, BAD_CAST "name") == 0)
        {
            ret = strdup((char*) xmlNodeGetContent(cur_node));
            break;
        }
    }

    /*free the document */
    xmlFreeDoc(doc);
    xmlCleanupParser();

    return ret;
}
