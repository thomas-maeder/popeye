#if !defined(PYOUTPUT_H)
#define PYOUTPUT_H

#include "py.h"
#include "stipulation/goals/goals.h"
#include "pystip.h"
#include "pytable.h"

#define ENUMERATION_TYPENAME output_mode
#define ENUMERATORS \
  ENUMERATOR(output_mode_tree), \
    ENUMERATOR(output_mode_line), \
    ENUMERATOR(output_mode_none)

#define ENUMERATION_DECLARE

#include "pyenum.h"


/* Set the output mode to be used for the current stipulation
 */
void set_output_mode(output_mode mode);

/* Instrument the stipulation structure with slices that implement
 * the selected output mode.
 * @param si identifies slice where to start
 */
void stip_insert_output_slices(slice_index si);

/* Write the appropriate amount of whitespace for the following output
 * to be correctely indented.
 */
void write_indentation(void);


/* Initialize based on the stipulation
 * @param si index of root of subtree that we initialise output for
 */
void init_output(slice_index si);

#endif
