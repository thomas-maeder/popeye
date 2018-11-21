#if !defined(INPUT_PLAINTEXT_PIECES_H)
#define INPUT_PLAINTEXT_PIECES_H

#include "position/position.h"
#include "utilities/boolean.h"

/* Identify a piece walk from the characters of its shortcut
 * @param a 1st character
 * @param b 2nd character (pass ' ' for 1 character shortcuts)
 * @return index of the shortcut in the language-dependent table of piece walk shortcuts
 *         nr_piece_walks if a and b don't represent a piece walk in the current language
 * @note the characters are treated independently from their case
 */
int GetPieNamIndex(char a, char b);

/* Parse a piece walk whose shortcut uses up an entire token
 * @param tok where to start parsing
 * @param result where to store the detected walk
 * @return start of subsequent token
 * @note assigns nr_piece_walks to *result parsing was not successful
 */
char *ParsePieceWalkToken(char *tok, piece_walk_type *result);

/* Parse a sequence of pieces including flags
 * @param tok where to start parsing
 * @return start of subsequent token
 */
char *ParsePieces(char *tok);

/* Try to parse a piece colour
 * @param tok where to start parsing
 * @param colour_is_mandatory is a colour indication mandatory or optional?
 * @return the detected colour (if any)
 */
Flags ParseColour(char *tok, boolean colour_is_mandatory);

/* Advance the input stream to the next token, then parse 0 to n non-colour piece flags.
 * @param flags where to save the detected flags
 * @return start of subsequent token
 */
char *ParsePieceFlags(Flags *flags);

/* Parse a piece walk
 * @param tok where to parse from
 * @param name where to write the detected walk to
 * @return position immediately behind the walk (no white space needed between
 *         walk and squares in pieces)
 * @note assigns nr_piece_walks to *walk if tok doesn't contain a
 *       recognisable piece walk shortcut
 */
char *ParsePieceWalk(char *tok, piece_walk_type *name);

#endif
