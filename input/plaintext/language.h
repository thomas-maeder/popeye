#if !defined(INPUT_PLAINTEXT_LANGUAGE_H)
#define INPUT_PLAINTEXT_LANGUAGE_H

/* Language identifiers
 */
typedef enum
{
  French,
  German,
  English,
  
  LanguageCount,
  LanguageDefault = French
} Language;

/* Language (hopefully) used by user
 */
extern Language UserLanguage;

#endif
