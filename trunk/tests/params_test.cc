#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>

#include <params-int.h>
#include <params-string.h>
#include <params-ipv4.h>
#include <params-enum.h>
#include <params-ipv6.h>

#include <string>
using namespace std;


string g_mgmt_path = ".";

void test_int(
	const char* str, 
	int use_min, long min,
	int use_max, long max,
	long expected_value, int expected_ret)
{
	long value;
	int ret;
	string err;
	
	ret = int_parse(
		str, &value,
		use_min, min,
		use_max, max, 
		&err);

	printf("parse_int: ret=%d out=%ld err=<%s>\n", ret, value, err.c_str());

	if (ret != expected_ret)
	{
		printf("parse_int: unexpected return value %d while expecting %d",
			ret, expected_ret);
		
		exit(-1);
	}
	
	if (value != expected_value)
	{
		printf("parse_int: unexpected value <%ld> while expecting <%ld>",
			value, expected_value);
		
		exit(-1);
	}
}

void unit_int()
{
	test_int("12", 0, 0, 0, 0, /* --> */ 12, 0);
	test_int("-12", 0, 0, 0, 0, /* --> */ -12, 0);
	test_int("0xABC", 0, 0, 0, 0, /* --> */ 0xABC, 0);
	test_int("-0xABC", 0, 0, 0, 0, /* --> */ -0xABC, 0);
	test_int("-0xABC", 0, 0, 0, 0, /* --> */ -0xABC, 0);
	test_int("0123", 0, 0, 0, 0, /* --> */ 0123, 0);
	test_int("-0123", 0, 0, 0, 0, /* --> */ -0123, 0);
	test_int("100", 0, 0, 1, 100, /* --> */ 100, 0);
	test_int("101", 0, 0, 1, 100, /* --> */ 0, -1);
	test_int("99", 0, 0, 1, 100, /* --> */ 99, 0);
	test_int("-999999", 0, 0, 1, 100, /* --> */ -999999, 0);
	test_int("0", 1, 0, 0, 0, /* --> */ 0, 0);
	test_int("-10", 1, -10, 0, 0, /* --> */ -10, 0);
	test_int("-12", 1, -10, 0, 0, /* --> */ 0, -1);
	
	// both min and max
	test_int("-10", 1, -10, 1, 40, /* --> */ -10, 0);
	test_int("40", 1, -10, 1, 40, /* --> */ 40, 0);
	test_int("-11", 1, -10, 1, 40, /* --> */ 0, -1);
	test_int("-41", 1, -10, 1, 40, /* --> */ 0, -1);
}

void test_str(
	const char* input,
	int use_min, long min,
	int use_max, long max,
	const char* expect,
	int expect_ret)
{
	string val;
	string err;
	int ret;
	
	ret = string_parse(input, &val, use_min, min, use_max, max, &err);
	printf("parse_string: ret=%d val=<%s> err=<%s>\n", ret, val.c_str(), err.c_str());

	if (ret != expect_ret)
	{
		printf("parse_string: expected ret %d returned %d\n", expect_ret, ret);
		exit(-1);
	}					
	
	if (ret == 0 && expect && strcmp(val.c_str(), expect) != 0)
	{
		printf("parse_string: expected <%s> received <%s>\n", expect, val.c_str());
		exit(-1);
	}
}

void unit_string()
{
	test_str("hello", 0, 0, 0, 0, "hello", 0);
	test_str("helloworld", 0, 0, 0, 0, "helloworld", 0);
	test_str("", 0, 0, 0, 0, "", 0);
	test_str("", 1, 0, 0, 0, "", 0);
	test_str("yoyo", 1, 0, 0, 0, "yoyo", 0);
	test_str("hel", 1, 3, 0, 0, "hel", 0);
	test_str("he", 1, 3, 0, 0, NULL, -1);
	test_str("h", 1, 3, 0, 0, NULL, -1);
	test_str("", 1, 3, 0, 0, NULL, -1);
	
	test_str("", 0, 0, 1, 0, "", 0);
	test_str("h", 0, 0, 1, 0, "h", -1);
	test_str("h", 0, 0, 1, 1, "h", 0);
	test_str("hello", 0, 0, 1, 10, "hello", 0);
	test_str("0123456789", 0, 0, 1, 10, "0123456789", 0);
	test_str("0123456789A", 0, 0, 1, 10, "0123456789A", -1);
	
	test_str("0123456789A", 1, 3, 1, 10, "0123456789A", -1);
	test_str("AB", 1, 3, 1, 10, "AB", -1);
	test_str("ABC", 1, 3, 1, 10, "ABC", 0);
	test_str("0123456789", 1, 3, 1, 10, "0123456789", 0);
	test_str("0123456", 1, 3, 1, 10, "0123456", 0);

	// put some prefix whitespace.
	test_str(" \t\n   0123456", 1, 3, 1, 10, "0123456", 0);
}

void test_ipv4(const char* input, unsigned char ex_0, unsigned char ex_1, unsigned char ex_2, unsigned char ex_3, int ex_ret)
{
	string err;
	ipv4_t out;
	int ret;

	ret = ipv4_parse(input, &out, &err);
	printf("parse_ipv4: ret=%d out=<%d.%d.%d.%d> err=<%s>\n", ret, out.as_bytes[0], out.as_bytes[1], out.as_bytes[2], out.as_bytes[3], err.c_str());

	if (ret != ex_ret)
	{
		printf("parse_ipv4: %d returned. expected %d\n", ret, ex_ret);
		exit(-1);
	}

	if (ret == 0 && 
		(out.as_bytes[0] != ex_0 ||
		 out.as_bytes[1] != ex_1 ||
		 out.as_bytes[2] != ex_2 ||
		 out.as_bytes[3] != ex_3))
	{
		printf("parse_ipv4: expected %d.%d.%d.%d", out.as_bytes[0], out.as_bytes[1], out.as_bytes[2], out.as_bytes[3]);
		exit(-1);
	}
}

void unit_ipv4()
{
	test_ipv4("1.2.3.4", 1,2,3,4, 0);
	test_ipv4("1.2.3.400", 1,2,3,4, -1);
	test_ipv4("1.2.300.4", 1,2,3,4, -1);
	test_ipv4("1.2000.3.4", 1,2,3,4, -1);
	test_ipv4("1000.2.3.4", 1,2,3,4, -1);
	test_ipv4("1000.2000.300.400", 1,2,3,4, -1);

	test_ipv4("255.255.255.255", 255,255,255,255, 0);
	test_ipv4("0.0.0.0", 0,0,0,0, 0);
	test_ipv4("256.255.255.255", 255,255,255,255, -1);
	test_ipv4("256.256.255.255", 255,255,255,255, -1);
	test_ipv4("256.255.256.255", 255,255,255,255, -1);
	test_ipv4("256.255.255.256", 255,255,255,255, -1);

	test_ipv4("250.255", 255,255,255,255, -1);
	test_ipv4("250.255.123", 255,255,255,255, -1);
	test_ipv4("123", 255,255,255,255, -1);
	test_ipv4("hello", 255,255,255,255, -1);
	test_ipv4("12.22.222.222", 12,22,222,222, 0);
	
	test_ipv4("0xa.0xb.0xc.0xd", 12,22,222,222, -1);
}

#include <vector>
using namespace std;

vector<string> to_string_vector(const char* array[], int count)
{
	vector<string> v;
	for (int i = 0; i < count; ++i)
	{
		v.push_back(string(array[i]));
	}
	return v;
}

void test_enum(const char* input, const char* values[], int value_count, int case_sens, int ex_res, int ex_ret)
{
	string err;
	int ret;
	int idx;
	ret =	enum_parse(input, to_string_vector(values, value_count), case_sens, &idx, &err);
	printf("parse_enum: ret=%d idx=%d (%s) err=<%s>\n", ret, idx, idx != -1 ? values[idx] : "<none>", err.c_str());

	if (ret != ex_ret)
	{
		printf("parse_enum: invalid ret. expected %d\n", ex_ret);
		exit(-1);
	}

	if (ret == 0 && ex_res != idx)
	{
		printf("parse enum: invalid value. expected %d\n", ex_res);
		exit(-1);
	}
}

void unit_enum()
{
	const char* values[] = { "apple", "orange", "lemon", "grape" };
	const char* novals[] = {};
	const char* onlyem[] = { "" };
	
	test_enum("apple", values, sizeof(values) / sizeof(char*), 0, 0, 0);
	test_enum("orange", values, sizeof(values) / sizeof(char*), 0, 1, 0);
	test_enum("lemon", values, sizeof(values) / sizeof(char*), 0, 2, 0);
	test_enum("grape", values, sizeof(values) / sizeof(char*), 0, 3, 0);
	
	test_enum("GRapE", values, sizeof(values) / sizeof(char*), 0, 3, 0);
	test_enum("GRapE", values, sizeof(values) / sizeof(char*), 1, 3, -1);
	test_enum("grape", values, sizeof(values) / sizeof(char*), 1, 3, 0);
	
	test_enum("grapes", values, sizeof(values) / sizeof(char*), 0, 3, -1);

	test_enum("", values, sizeof(values) / sizeof(char*), 0, 3, -1);
	test_enum("grape and me", values, sizeof(values) / sizeof(char*), 0, 3, -1);
	
	test_enum("grape", novals, sizeof(novals) / sizeof(char*), 0, 3, -1);
	test_enum("", novals, sizeof(novals) / sizeof(char*), 0, 3, -1);
	test_enum("", onlyem, sizeof(onlyem) / sizeof(char*), 0, 0, 0);
}


/*
typedef union
{
	unsigned short as_shorts[8];
	unsigned char as_bytes[16];
} ipv6_t;


#include <regex.h>
#include <string.h>

#define IPV6_PATTERN "^((([0-9A-Fa-f]{1,4}:){7}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){6}:[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){5}:([0-9A-Fa-f]{1,4}:)?[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){4}:([0-9A-Fa-f]{1,4}:){0,2}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){3}:([0-9A-Fa-f]{1,4}:){0,3}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){2}:([0-9A-Fa-f]{1,4}:){0,4}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){6}((\\b((25[0-5])|(1[0-9]{2})|(2[0-4][0-9])|([0-9]{1,2}))\\b)\\.){3}(\\b((25[0-5])|(1[0-9]{2})|(2[0-4][0-9])|([0-9]{1,2}))\\b))|(([0-9A-Fa-f]{1,4}:){0,5}:((\\b((25[0-5])|(1[0-9]{2})|(2[0-4][0-9])|([0-9]{1,2}))\\b)\\.){3}(\\b((25[0-5])|(1[0-9]{2})|(2[0-4][0-9])|([0-9]{1,2}))\\b))|(::([0-9A-Fa-f]{1,4}:){0,5}((\\b((25[0-5])|(1[0-9]{2})|(2[0-4][0-9])|([0-9]{1,2}))\\b)\\.){3}(\\b((25[0-5])|(1[0-9]{2})|(2[0-4][0-9])|([0-9]{1,2}))\\b))|([0-9A-Fa-f]{1,4}::([0-9A-Fa-f]{1,4}:){0,5}[0-9A-Fa-f]{1,4})|(::([0-9A-Fa-f]{1,4}:){0,6}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){1,7}:))$"


//#define IPV6_PATTERN "^((([0-9A-Fa-f]{1,4}:){7}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){6}:[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){5}:([0-9A-Fa-f]{1,4}:)?[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){4}:([0-9A-Fa-f]{1,4}:){0,2}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){3}:([0-9A-Fa-f]{1,4}:){0,3}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){2}:([0-9A-Fa-f]{1,4}:){0,4}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){6}((\\b((25[0-5])|(1\\d{2})|(2[0-4]\\d)|(\\d{1,2}))\\b)\\.){3}(\\b((25[0-5])|(1\\d{2})|(2[0-4]\\d)|(\\d{1,2}))\\b))|(([0-9A-Fa-f]{1,4}:){0,5}:((\\b((25[0-5])|(1\\d{2})|(2[0-4]\\d)|(\\d{1,2}))\\b)\\.){3}(\\b((25[0-5])|(1\\d{2})|(2[0-4]\\d)|(\\d{1,2}))\\b))|(::([0-9A-Fa-f]{1,4}:){0,5}((\\b((25[0-5])|(1\\d{2})|(2[0-4]\\d)|(\\d{1,2}))\\b)\\.){3}(\\b((25[0-5])|(1\\d{2})|(2[0-4]\\d)|(\\d{1,2}))\\b))|([0-9A-Fa-f]{1,4}::([0-9A-Fa-f]{1,4}:){0,5}[0-9A-Fa-f]{1,4})|(::([0-9A-Fa-f]{1,4}:){0,6}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){1,7}:))$"

#define IPV6_MAX_MATCH	256
*/

/**
 * converts a string to ipv6 type. assume string is a valid ipv6 string.
 */

/*
void _string_to_ipv6(const char* str, ipv6_t* out_value)
{
	char* lasts;
	char* tok;
	char* input_copy;
	char* p;
	char* last_tok_end;
	long parts[9];
	int part;
	int i;
	int comp_size;
	int comp_start = -1;

	assert(str != NULL);
	assert(out_value != NULL);
				
	input_copy = malloc(strlen(str) + 1);
	strcpy(input_copy, str);
	
	p = input_copy;
	
	// reset parts
	for (part = 0; part < 6; ++part) parts[part] = -1;

	part = 0;
	last_tok_end = p - 1;
	while ((tok = strtok_r(p, ":", &lasts)) != NULL)
	{
		int sep_len;
		
		p = NULL;
		sep_len = tok - last_tok_end;

		// if we had '::', we are putting a marker to indicate compression.
		if (sep_len > 1)
		{
			parts[part] = -2;
			comp_start = part;
			part++;
		}
		
		// adding the part.
		parts[part] = strtol(tok, NULL, 16);
		part++;
		
		last_tok_end = tok + strlen(tok);
	}

	// calculate compression size.
	if (comp_start != -1)	comp_size = 9 - part;
	else comp_size = -1;
	
	// store parts in out_value, while storing 0x0000 in compression part.
	for (part = 0, i = 0; i < 8; ++i)
	{
		if (i >= comp_start && i < comp_start + comp_size)
			out_value->as_shorts[i] = 0x0000;
		else
			out_value->as_shorts[i] = parts[part++];

		if (i == comp_start) part++;
	}
	
	free(input_copy);
}
*/

/**
 * parses a string as an ip version y string.
 * @param input - the input string.
 * @param out_value - the output ipvy address.
 * @param out_err - if not null, returns a descriptive error.
 * @param err_len - size of out_err.
 * @return 0 upon success, -1 on failure.
 */
/*
int parse_ipv6(
	const char* input,
	ipv6_t* out_value,
	char* out_err,
	int err_len)
{
	regmatch_t matches[IPV6_MAX_MATCH];
	int ret;
	regex_t re;
	int reg_ret;
	int i;
	
	assert(out_value != NULL);
	
	// reset output value and error.
	for (i = 0; i < 8; ++i) out_value->as_shorts[i] = 0;
	if (out_err && err_len > 0) *out_err = '\0';
	
	// compile the regular expression.
	reg_ret = regcomp(&re, IPV6_PATTERN, REG_EXTENDED);
	if (reg_ret != 0)
	{
		if (out_err) snprintf(out_err, err_len, "unable to parse ipv6 address. invalid regular expression used");
		ret = -1;
		goto cleanup;
	}

	// execute the regular expression on the input string.
	reg_ret = regexec(&re, input, IPV6_MAX_MATCH, matches, 0);
	if (reg_ret == REG_NOMATCH)
	{
		if (out_err) snprintf(out_err, err_len, "'%s': invalid ipv6 address", input);
		ret = -1;
		goto cleanup;
	}

	_string_to_ipv6(input, out_value);
	
	ret = 0;
	
cleanup:
	return ret;
}

*/
void test_ipv6(
	const char* input, int ex_ret, 
	unsigned short e0, 
	unsigned short e1, 
	unsigned short e2, 
	unsigned short e3, 
	unsigned short e4, 
	unsigned short e5, 
	unsigned short e6, 
	unsigned short e7)
{
	int ret;
	string err;
	ipv6_t value;
	int i;

	ret = ipv6_parse(input, &value, &err);
	printf("<%s> ret=%d value=<", input, ret);
	for (i = 0; i < 8; ++i) printf("%x;", value.as_shorts[i]);
	printf("> err=<%s>\n", err.c_str());
	
	if (ret != ex_ret)
	{
		printf("parse_ipv6: unexpected ret. expecting %d\n", ex_ret);
		exit(-1);
	}

	if (ret == 0 && (
		value.as_shorts[0] != e0 ||
		value.as_shorts[1] != e1 ||
		value.as_shorts[2] != e2 ||
		value.as_shorts[3] != e3 ||
		value.as_shorts[4] != e4 ||
		value.as_shorts[5] != e5 ||
		value.as_shorts[6] != e6 ||
		value.as_shorts[7] != e7))
	{
		printf("parse_ipv6: unexpected output (expecting %x;%x;%x;%x;%x;%x;%x;%x).\n",
										e0,e1,e2,e3,e4,e5,e6,e7);
		exit(-1);
	}
}

void test_ipv6_good(const char* input, short e0, short e1, short e2, short e3, short e4, short e5, short e6, short e7)
{
	test_ipv6(input, 0, e0, e1, e2, e3, e4, e5, e6, e7);
}

void test_ipv6_bad(const char* input)
{
	test_ipv6(input, -1, 0, 0, 0, 0, 0, 0, 0, 0);
}



void unit_ipv6()
{
	test_ipv6_good("FEDC:BA98:7654:3210:FEDC:BA98:7654:3210", 
									0xfedc,0xba98,0x7654,0x3210,0xfedc,0xba98,0x7654,0x3210);

	test_ipv6_good("1080:0:0:0:8:800:200C:417A",
									0x1080,0,0,0,8,0x800,0x200c,0x417a);
	test_ipv6_good("0:0:0:0:0:0:0:1", 0,0,0,0,0,0,0,1);
	
	test_ipv6_good("1080::8:800:200C:417a",0x1080,0,0,0,0x8,0x800,0x200c,0x417a);
	
	test_ipv6_good("::ffff:129:144:52:38", 0,0,0,0xffff,0x129,0x144,0x52,0x38);
	test_ipv6_good("AB::CD", 0xab, 0,0,0,0,0,0, 0xcd);
	test_ipv6_bad("128.0.0.1");
	test_ipv6_bad("FEDC:BA98:7654:3210:FEDC:BA98:7654:3210:AA31");
	test_ipv6_good("::1", 0, 0, 0, 0, 0, 0, 0, 1);

	test_ipv6_bad("FEDC::7654:3210::BA98:7654:3210");
	test_ipv6_bad("FEDC:BA98:7654:3210");
	test_ipv6_bad("::");
	
}


int main(int argc, char* argv[])
{
	unit_int();
	unit_string();
	unit_ipv4();
	unit_enum();
	unit_ipv6();

	return 0;
}
