/* Generator for an enum type and an associated array of character
 * strings containing the enumerator names.
 *
 * Inspired by several contributions to Internet discussion groups
 * (thanks!)

 * Example:

 * This:

#define ENUMERATION_TYPENAME enum_type
#define ENUMERATORS \
  ENUMERATOR(enumerator0)     \
    , ENUMERATOR(enumerator1) \
    , ENUMERATOR(nr_enum_type) \
    , ASSIGNED_ENUMERATOR(no_enum_type=nr_enum_type)

#define ENUMERATION_DECLARE

#include "utilities/enumeration.h"

 * generates the following definition:

typedef enum
{
 enumerator0 , enumerator1 , nr_enum_type , no_enum_type=nr_enum_type
} enum_type;

extern char const *enum_type_names[];

  * And this:


#define ENUMERATION_TYPENAME enum_type
#define ENUMERATORS \
  ENUMERATOR(enumerator0)     \
    , ENUMERATOR(enumerator1) \
    , ENUMERATOR(nr_enum_type) \
    , ASSIGNED_ENUMERATOR(no_enum_type=nr_enum_type)

#define ENUMERATION_MAKESTRINGS

#include "utilities/enumeration.h"

  * the following:

char const *enum_type_names[] =
{
 "enumerator0" , "enumerator1" , "nr_enum_type" ,
};

*/

#if defined(ENUMERATION_DECLARE)

/* Define the enumeration type named ENUMERATION_TYPENAME
 */
#define ENUMERATOR(x) x
#define ASSIGNED_ENUMERATOR(x) x

typedef enum
{
	ENUMERATORS
} ENUMERATION_TYPENAME;

#undef ASSIGNED_ENUMERATOR
#undef ENUMERATOR
#undef ENUMERATION_DECLARE

/* Declare the associated character string array
 * (definition of the array see below)
 */
#define ENUMNAME_CONCAT2(a,b) a##b
#define ENUMNAME_CONCAT(a,b) ENUMNAME_CONCAT2(a,b)
extern char const *ENUMNAME_CONCAT(ENUMERATION_TYPENAME,_names)[];
#undef ENUMNAME_CONCAT
#undef ENUMNAME_CONCAT2

#endif

#if defined(ENUMERATION_MAKESTRINGS)

/* Define the associated character string array with 1 element per
 * ENUMERATORS (and no element per ASSIGNED_ENUMERATOR)
 */
#define ENUMNAME_CONCAT2(a,b) a##b
#define ENUMNAME_CONCAT(a,b) ENUMNAME_CONCAT2(a,b)
#define ENUMERATOR(x) #x
#define ASSIGNED_ENUMERATOR(x) ""
char const *ENUMNAME_CONCAT(ENUMERATION_TYPENAME,_names)[] =
{
	ENUMERATORS
};
#undef ASSIGNED_ENUMERATOR
#undef ENUMERATOR
#undef ENUMNAME_CONCAT
#undef ENUMNAME_CONCAT2
#undef ENUMERATION_MAKESTRINGS

#endif


/* Undefine the parameter macros
 */
#undef ENUMERATION_TYPENAME
#undef ENUMERATORS
