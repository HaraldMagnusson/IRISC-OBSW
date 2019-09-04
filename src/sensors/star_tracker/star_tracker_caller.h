/* -----------------------------------------------------------------------------
 * Component Name: Star Tracker Caller
 * Parent Component: Star Tracker
 * Author(s): Niklas Ulfvarson
 * Purpose: This is the interface to run the Tetra python program from the 
 * 			main OBSW.
 * Usage: A float array of length 4 i passed, and values from the star tracker 
 *          is returned in this array in order:
 *          0: RA
 *          1: Dec
 *          2: Roll
 *          3: FoV
 *          
 *          The name of the parameter is printed by tetra, but then ignored.
 *          If no attitude could be calculated all of these will be 0. This 
 *          can obviously not happen if an attitude is calculated, as FoV will
 *          always have a positive non-zero value.
 *
 * -----------------------------------------------------------------------------
 */

void iriscTetra(float stReturn[]);
