#ifdef LIBYAML_AVAILABLE
#include "dict.h"
#include "portable.h"
#include "filesystem.h"
#include <stdlib.h>
#include <yaml.h>
//#include <jsmn.h>
#include <assert.h>

dict_t * _d_yaml_to_dict(yaml_document_t * doc, yaml_node_t * node)
{
	if(!doc || !node)
		return NULL;

	switch(node->type)
	{
	case YAML_SCALAR_NODE:
	{
		dict_t * dict = calloc(1, sizeof(dict_t));
		dict->count = node->data.scalar.length;
		char * value = calloc(1, dict->count + 1);
		dict->value = value;
		memcpy(value, node->data.scalar.value, dict->count);
		return dict;
	}
	case YAML_SEQUENCE_NODE:
	{
		if(node->data.sequence.items.start != node->data.sequence.items.top)
		{
			dict_t * dict = calloc(1, sizeof(dict_t));
			dict->count = node->data.sequence.items.top - node->data.sequence.items.start;
			dict->array = calloc(1, dict->count * sizeof(dict_t*));
			for(size_t i = 0; i < dict->count; ++i)
				dict->array[i] = _d_yaml_to_dict(doc, yaml_document_get_node(doc, *(node->data.sequence.items.start + i)));
			return dict;
		}
		else
			return NULL;
	}
	case YAML_MAPPING_NODE:
	{
		if(node->data.mapping.pairs.start != node->data.mapping.pairs.top)
		{
			dict_t * dict = calloc(1, sizeof(dict_t));
			dict->count = node->data.mapping.pairs.top - node->data.mapping.pairs.start;
			dict->dict = kh_init_dict_map();

			for(size_t i = 0; i < dict->count; ++i)
			{
				yaml_node_pair_t * pair = node->data.mapping.pairs.start + i;

				yaml_node_t * key = yaml_document_get_node(doc, pair->key);

				if(!key || key->type != YAML_SCALAR_NODE)
				{
					printf("hey, only string types are supported as key in yaml hashmaps, check line %i\n", key ? (int)key->start_mark.line : -1);
					continue;
				}

				char * key_value = calloc(1, key->data.scalar.length + 1);
				memcpy(key_value, key->data.scalar.value, key->data.scalar.length);

				dict_t * value = _d_yaml_to_dict(doc, yaml_document_get_node(doc, pair->value));

				int retk;
				khint_t k = kh_put_dict_map(dict->dict, key_value, &retk);
				kh_value(dict->dict, k) = value;
			}
			return dict;
		}
		else
			return NULL;
	}
	default:
		return NULL;
	}
}

dict_t * dparsey(const char * yaml_filename)
{
	FILE * f = fsopen(yaml_filename, "r");
	if(!f)
		return NULL;

	yaml_parser_t yp;
	if(!yaml_parser_initialize(&yp))
		return NULL;

	yaml_parser_set_input_file(&yp, f);
	yaml_document_t doc;
	dict_t * ret = NULL;

	// TODO use token loading lol
	if(yaml_parser_load(&yp, &doc))
	{
		ret = _d_yaml_to_dict(&doc, yaml_document_get_root_node(&doc));
		yaml_document_delete(&doc);
	}

	yaml_parser_delete(&yp);
	fclose(f);

	return ret;
}

#if 0
dict_t * _d_jsmn_to_dict(const char * js, jsmntok_t * tokens, size_t count, size_t * pos)
{
	if(*pos > count)
		return NULL;

	jsmntok_t * t = tokens + (*pos);
	(*pos)++;

	if(t->type == 0)
		return NULL;

	dict_t * dict = calloc(1, sizeof(dict_t));
	dict->count = t->size;

	switch(t->type)
	{
	case JSMN_OBJECT:
	{
		#ifndef JSMN_STRICT
		#error please enable JSMN_STRICT
		#endif
		// with JSMN_STRICT enabled its safe to assume that count if count of pairs
		// because jsmn have strange counting thing where key gets additional size
		dict->dict = kh_init_dict_map();
		for(size_t i = 0; i < dict->count; ++i)
		{
			dict_t * key = _d_jsmn_to_dict(js, tokens, count, pos);
			assert(key->value);

			dict_t * value = _d_jsmn_to_dict(js, tokens, count, pos);
			assert(value);

			int retk;
			khint_t k = kh_put_dict_map(dict->dict, key->value, &retk);
			kh_value(dict->dict, k) = value;
		}
		break;
	}
	case JSMN_ARRAY:
		dict->array = calloc(1, dict->count * sizeof(dict_t*));
		for(size_t i = 0; i < dict->count; ++i)
		{
			dict_t * value = _d_jsmn_to_dict(js, tokens, count, pos);
			assert(value);
			dict->array[i] = value;
		}
		break;
	case JSMN_STRING:
	case JSMN_PRIMITIVE:
	{
		dict->count = t->end - t->start;
		char * value = calloc(1, dict->count + 1);
		dict->value = value;
		memcpy(value, js + t->start, dict->count);
		break;
	}
	default:
		break;
	}
	return dict;
}
#endif

#if 0
dict_t * dparsejs(const char * json_string)
{
	jsmn_parser jp = {0};
	jsmn_init(&jp);

	size_t len = strlen(json_string);
	int count = jsmn_parse(&jp, json_string, len, NULL, 0);

	if(count < 0)
		return NULL;

	size_t size = count * sizeof(jsmntok_t);
	if(size > 2 * 1024 * 1024) // meh, limit the size
		return NULL;

	#ifdef _MSC_VER
	#define alloca _alloca
	#endif

	jsmntok_t * t = (jsmntok_t*)alloca(size);

	jsmn_init(&jp);
	int count2 = jsmn_parse(&jp, json_string, len, t, count);

	if(count != count2)
		return NULL;

	//for(size_t i = 0; i < count; ++i)
	//	printf("%i %i %i->%i=%i\n", i, t[i].type, t[i].start, t[i].end, t[i].size);

	size_t pos = 0;
	dict_t * r =  _d_jsmn_to_dict(json_string, t, count, &pos);
	//dtraverse(r, 0);
	return r;
}
#endif

void dfree(dict_t * dict)
{
	if(!dict)
		return;

	if(dict->value)
		free(dict->value);

	if(dict->array)
	{
		for(size_t i = 0; i < dict->count; ++i)
			dfree(dict->array[i]);
		free(dict->array);
	}

	if(dict->dict)
	{
		for(khint_t k = kh_begin(dict->dict); k != kh_end(dict->dict); ++k)
		{
			if(kh_exist(dict->dict, k))
			{
				free((char*)kh_key(dict->dict, k));
				dfree(kh_value(dict->dict, k));
			}
		}
		kh_destroy_dict_map(dict->dict);
	}

	free(dict);
}

void dtraverse(dict_t * dict, int level)
{
	printf("%*s", level * 4, "");
	if(dict->value)
		printf("%s\n", dict->value);
	else if(dict->array)
	{
		printf("array of %i\n", (int)dict->count);
		for(size_t i = 0; i < dict->count; ++i)
			dtraverse(dict->array[i], level + 1);
	}
	else if(dict->dict)
	{
		printf("dict of %i\n", (int)dict->count);

		for(khint_t k = kh_begin(dict->dict); k != kh_end(dict->dict); ++k)
		{
			if(kh_exist(dict->dict, k))
			{
				printf("%*s%s\n", (level + 1) * 4, "", kh_key(dict->dict, k));
				dtraverse(kh_value(dict->dict, k), level + 1);
			}
		}
	}
}

dict_t * dget(dict_t * dict, const char * key)
{
	if(!dict)
		return NULL;
	assert(dict->dict);
	int k = kh_get_dict_map(dict->dict, key);
	return (k != kh_end(dict->dict)) ? kh_value(dict->dict, k) : NULL;
}
dict_t * dgeti(dict_t * dict, size_t index)
{
	if(!dict)
		return NULL;
	assert(dict->array);
	return (index < dict->count) ? dict->array[index] : NULL;
}

diter_t      dibegin(dict_t * dict) {return dict && dict->dict ? kh_begin(dict->dict) : 0;}
diter_t      diend(dict_t * dict)   {return dict && dict->dict ? kh_end(dict->dict) : 0;}
const char * dikey(dict_t * dict, diter_t index)    {return dict && dict->dict && kh_exist(dict->dict, index) ? kh_key(dict->dict, index) : NULL;}
dict_t *     divalue(dict_t * dict, diter_t index)  {return dict && dict->dict && kh_exist(dict->dict, index) ? kh_value(dict->dict, index) : NULL;}

const char * dstr(dict_t * dict, const char * default_value) {return dict && dict->value ?      dict->value  : default_value;}
int          dint(dict_t * dict, int default_value)          {return dict && dict->value ? atoi(dict->value) : default_value;}
uint32_t     duint32(dict_t * dict, uint32_t default_value)  {return dict && dict->value ? strtoul(dict->value, NULL, 10) : default_value;}
uint64_t     duint64(dict_t * dict, uint64_t default_value)  {return dict && dict->value ? strtoull(dict->value, NULL, 10) : default_value;}
float        dfloat(dict_t * dict, float default_value)      {return dict && dict->value ? (float)atof(dict->value) : default_value;}
const char * dgstr(dict_t * dict, const char * key, const char * default_value) {return dstr(   dget(dict, key), default_value);}
int          dgint(dict_t * dict, const char * key, int default_value)          {return dint(   dget(dict, key), default_value);}
uint32_t     dguint32(dict_t * dict, const char * key, uint32_t default_value)  {return duint32(dget(dict, key), default_value);}
uint64_t     dguint64(dict_t * dict, const char * key, uint64_t default_value)  {return duint64(dget(dict, key), default_value);}
float        dgfloat(dict_t * dict, const char * key, float default_value)      {return dfloat( dget(dict, key), default_value);}
bool         dgstr2(char * buffer, size_t buffer_length, dict_t * dict, const char * key, const char * default_value)
{
	return strlcpy(buffer, dgstr(dict, key, default_value), buffer_length) < buffer_length;
}
#endif
