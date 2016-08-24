#include "conditions/circe/rebirth_square.h"
#include "conditions/circe/circe.h"
#include "stipulation/pipe.h"
#include "debugging/assert.h"
#include "debugging/trace.h"

/* Determine the rebirth square determinator of a Circe variant
 * @param variant address of the structure representing the variant
 * @return determinator slice type
 */
static slice_type get_rebirth_square_determinator(circe_variant_type const *variant)
{
  slice_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  switch (variant->determine_rebirth_square)
  {
    case circe_determine_rebirth_square_from_pas:
    case circe_determine_rebirth_square_rank:
      result = STCirceDetermineRebirthSquare;
      break;

    case circe_determine_rebirth_square_super:
    case circe_determine_rebirth_square_cage:
      result = STSuperCirceDetermineRebirthSquare;
      break;

    case circe_determine_rebirth_square_take_and_make:
      result = STTakeMakeCirceDetermineRebirthSquares;
      break;

    case circe_determine_rebirth_square_antipodes:
      result = STAntipodesCirceDetermineRebirthSquare;
      break;

    case circe_determine_rebirth_square_pwc:
      result = STPWCDetermineRebirthSquare;
      break;

    case circe_determine_rebirth_square_file:
      result = STFileCirceDetermineRebirthSquare;
      break;

    case circe_determine_rebirth_square_diagram:
      result = STDiagramCirceDetermineRebirthSquare;
      break;

    case circe_determine_rebirth_square_symmetry:
      result = STSymmetryCirceDetermineRebirthSquare;
      break;

    case circe_determine_rebirth_square_equipollents:
      result = (variant->relevant_side_overrider==circe_relevant_side_overrider_mirror
                ? STCirceContraparrainDetermineRebirth
                : STCirceParrainDetermineRebirth);
      break;

    default:
      assert(0);
      result = no_slice_type;
      break;
  }

  TraceFunctionExit(__func__);
  TraceEnumerator(slice_type,result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine the rebirth square adaptor (if any) of a Circe variant
 * @param variant address of the structure representing the variant
 * @return determinator slice type; no_slice_type if there isn't any
 */
static slice_type get_rebirth_square_adaptor(circe_variant_type const *variant)
{
  slice_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  switch (variant->rebirth_square_adapter)
  {
    case circe_rebirth_square_adapter_diametral:
      result = STCirceDiametralAdjustRebirthSquare;
      break;

    case circe_rebirth_square_adapter_verticalmirror:
      result = STCirceVerticalMirrorAdjustRebirthSquare;
      break;

    case circe_rebirth_square_adapter_rank:
      result = STRankCirceProjectRebirthSquare;
      break;

    case circe_rebirth_square_adapter_frischauf:
      result = STCirceFrischaufAdjustRebirthSquare;
      break;

    default:
      result = no_slice_type;
      break;
  }

  TraceFunctionExit(__func__);
  TraceEnumerator(slice_type,result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument the solving machinery with the logic for determining the rebirth
 * square in a Circe variant
 * @param si root slice of the solving machinery
 * @param variant address of the structure representing the Circe variant
 * @param interval_start start of the slice sequence representing the variant
 */
void circe_solving_instrument_rebirth_square(slice_index si,
                                             struct circe_variant_type const *variant,
                                             slice_type interval_start)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",interval_start);
  TraceFunctionParamListEnd();


  circe_instrument_solving(si,
                           interval_start,
                           STCirceDeterminingRebirth,
                           alloc_pipe(get_rebirth_square_determinator(variant)));

  {
    slice_type const adaptor = get_rebirth_square_adaptor(variant);
    if (adaptor!=no_slice_type)
      circe_instrument_solving(si,
                               interval_start,
                               STCirceDeterminingRebirth,
                               alloc_pipe(adaptor));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Override the method for determining the rebirth square of a Circe variant object
 * @param adapter the overrider
 * @return true if it hasn't been overridden yet
 */
boolean circe_override_determine_rebirth_square(circe_variant_type *variant,
                                                circe_determine_rebirth_square_type determine)
{
  boolean result;

  if (variant->determine_rebirth_square==circe_determine_rebirth_square_from_pas)
  {
    variant->determine_rebirth_square = determine;
    result = true;
  }
  else
    result = false;

  return result;
}

/* Override the rebirth square adapter of a Circe variant object
 * @param adapter the overrider
 * @return true if the adapter hasn't been overridden yet
 */
boolean circe_override_rebirth_square_adapter(circe_variant_type *variant,
                                              circe_rebirth_square_adapter_type adapter)
{
  boolean result;

  if (variant->rebirth_square_adapter==circe_rebirth_square_adapter_none)
  {
    variant->rebirth_square_adapter = adapter;
    result = true;
  }
  else
    result = false;

  return result;
}
