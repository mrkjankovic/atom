/*    
 * Copyright (c) 2014, K. Kumar (me@kartikkumar.com)
 * All rights reserved.
 */

#ifndef ATOM_SOLVER_H
#define ATOM_SOLVER_H

#include <sstream>
#include <string>
#include <utility>

#include <gsl/gsl_multiroots.h>
#include <gsl/gsl_vector.h>

#include <libsgp4/DateTime.h>
#include <libsgp4/Eci.h>
#include <libsgp4/SGP4.h> 
#include <libsgp4/Tle.h>

#include "Atom/convertCartesianStateToTwoLineElements.hpp"

namespace atom
{

//! Execute Atom solver.
/*!
 * Executes Atom solver to find the transfer orbit connecting two positions. The epoch of the 
 * departure position and the Time-of-flight need to be specified.
 *
 * The Atom solver is an analog of the Lambert solver (Lancaster and Blanchard, 1969;
 * Gooding, 1990; Izzo, 2014), that aims to find the conic section that bridges two positions, at
 * given epochs, by using impulsive manveuvers (Delta-V maneuvers) at departure and arrival. The
 * Atom solver aims to solver a similar orbital transfer, subject to perturbations. The 
 * perturbations taken into account are those encoded in the SGP4/SDP4 propagators (Vallado, 2006).
 *
 * Since the Atom solver makes use fo the SGP4/SDP4 propagators, it can currently only solve for
 * perturbed transfers around the Earth. As a result, the Earth's gravitational parameter is fixed,
 * as specified by the SGP4/SDP4 propagators (Vallado, 2006).
 * 
 * Details of the underlying non-linear system and algorithm are catalogued by 
 * Kumar, et al. (2014).
 *
 * @sa     convertCartesianStateToTwoLineElements
 * @tparam Integer                     Type for integers 
 * @tparam Real                        Type for reals
 * @tparam Vector                      Type for vector of reals
 * @param  departurePosition           Cartesian position vector at departure [km]
 * @param  departureEpoch              Modified Julian Date (MJD) of departure
 * @param  arrivalPosition             Cartesian position vector at arrival [km]
 * @param  timeOfFlight                Time-of-Flight for orbital transfer [s]
 * @param  departureVelocityGuess      Initial guess for the departure velocity (serves as initial
 *                                     guess for the internal root-finding procedure) [km/s]
 * @param  solverStatusSummary         Status of non-linear solver printed as a table 
 * @param  numberOfIterations          Number of iterations completed by solver 
 * @param  referenceTle                Reference Two Line Elements [default: 0-TLE]
 * @param  earthGravitationalParameter Earth gravitational parameter [km^3 s^-2] [default: mu_SGP]
 * @param  earthMeanRadius             Earth mean radius [km] [default: R_SGP]
 * @param  absoluteTolerance           Absolute tolerance used to check if root-finder has 
 *                                     converged [default: 1.0e-10] (see Kumar, et al. (2014) for
 *                                     details on how convergence is tested)
 * @param  relativeTolerance           Relative tolerance used to check if root-finder has 
 *                                     converged [default: 1.0e-5] (see Kumar, et al. (2014) for 
 *                                     details on how convergence is tested)
 * @param  maximumIterations           Maximum number of solver iterations permitted. Once the 
 *                                     solver reaches this limit, the loop will be broken and the 
 *                                     solver status will report that it has not converged 
 *                                     [default: 100].
 * @return                             Departure and arrival velocities (stored in that order)
 */
template< typename Integer, typename Real, typename Vector >
const std::pair< Vector, Vector > executeAtomSolver( 
    const Vector& departurePosition, 
    const DateTime departureEpoch,
    const Vector& arrivalPosition, 
    const Real timeOfFlight, 
    const Vector& departureVelocityGuess,
    std::string& solverStatusSummary,
    Integer& numberOfIterations,
    const Tle referenceTle = Tle( ),
    const Real earthGravitationalParameter = kMU,
    const Real earthMeanRadius = kXKMPER,    
    const Real absoluteTolerance = 1.0e-10,
    const Real relativeTolerance = 1.0e-5,
    const Integer maximumIterations = 100 );

//! Execute Atom solver.
/*!
 * Executes Atom solver to find the transfer orbit connecting two positions. The epoch of the 
 * departure position and the Time-of-flight need to be specified.
 *
 * The Atom solver is an analog of the Lambert solver (Lancaster and Blanchard, 1969;
 * Gooding, 1990; Izzo, 2014), that aims to find the conic section that bridges two positions, at
 * given epochs, by using impulsive manveuvers (Delta-V maneuvers) at departure and arrival. The
 * Atom solver aims to solver a similar orbital transfer, subject to perturbations. The 
 * perturbations taken into account are those encoded in the SGP4/SDP4 propagators (Vallado, 2006).
 *
 * Since the Atom solver makes use fo the SGP4/SDP4 propagators, it can currently only solve for
 * perturbed transfers around the Earth. As a result, the Earth's gravitational parameter is fixed,
 * as specified by the SGP4/SDP4 propagators (Vallado, 2006).
 * 
 * Details of the underlying non-linear system and algorithm are catalogued by 
 * Kumar, et al. (2014).
 *
 * This is a function overload to ensure that the user can opt to leave out solver summary status
 * string and number of iterations counter from the call (overload is necessary since non-const
 * references cannot be assigned default values in C++).
 *
 * @sa     convertCartesianStateToTwoLineElements
 * @tparam Integer                Type for integers 
 * @tparam Real                   Type for reals
 * @tparam Vector                 Type for vector of reals
 * @param  departurePosition      Cartesian position vector at departure [km]
 * @param  departureEpoch         Modified Julian Date (MJD) of departure
 * @param  arrivalPosition        Cartesian position vector at arrival [km]
 * @param  timeOfFlight           Time-of-Flight for orbital transfer [s]
 * @param  departureVelocityGuess Initial guess for the departure velocity (serves as initial
 *                                guess for the internal root-finding procedure) [km/s]
 * @return                        Departure and arrival velocities (stored in that order)
 */
template< typename Integer, typename Real, typename Vector >
const std::pair< Vector, Vector > executeAtomSolver( 
    const Vector& departurePosition, 
    const DateTime departureEpoch,
    const Vector& arrivalPosition, 
    const Real timeOfFlight, 
    const Vector& departureVelocityGuess );

//! Compute residuals to execute Atom solver.
/*!
 * Evaluates system of non-linear equations and computes residuals to execute the Atom solver. The
 * residual function, \f$R\f$ is computed as follows:
 * The system of non-linear equations used is:
 *  \f[ 
 *      R = 0 = \bar{r}^{arrival,computed} - \bar{r}^{arrival,target}
 *  \f]
 * where \f$\bar{r}^{arrival,computed}\f$ is the Cartesian position computed by propagating the 
 * initial, prescribed state under the action of an initial impulsive Delta V, by a prescribed 
 * time-of-flight, and \f$\bar{x}^{arrival,target}\f$ is the target Cartesian position. Note that 
 * the residuals are used to drive a root-finding process that uses the GSL library.
 *
 * @sa executeAtomSolver
 * @tparam Integer              Type for integers 
 * @tparam Real                 Type for reals
 * @tparam Vector               Type for vector of reals
 * @param  independentVariables Vector of independent variables used by the root-finder
 * @param  parameters           Parameters required to compute the objective function
 * @param  residuals            Vector of computed residuals
 * @return                      GSL flag indicating success or failure
 */
template< typename Integer, typename Real, typename Vector >
int computeAtomResiduals( const gsl_vector* independentVariables,
                          void* parameters, 
                          gsl_vector* residuals );

//! Parameter struct used by Atom residual function.
/*!
 * Data structure with parameters used to compute Atom residual function.
 *
 * @sa computeAtomResiduals
 * @tparam Integer Type for integers  
 * @tparam Real    Type for reals
 * @tparam Vector  Type for vector of reals
 */
template< typename Integer, typename Real, typename Vector >
struct AtomParameters;

//! Execute Atom solver.
template< typename Integer, typename Real, typename Vector >
const std::pair< Vector, Vector > executeAtomSolver( 
    const Vector& departurePosition, 
    const DateTime departureEpoch,
    const Vector& arrivalPosition, 
    const Real timeOfFlight, 
    const Vector& departureVelocityGuess,
    std::string& solverStatusSummary,
    Integer& numberOfIterations,
    const Tle referenceTle,
    const Real earthGravitationalParameter,
    const Real earthMeanRadius,    
    const Real absoluteTolerance,
    const Real relativeTolerance,
    const Integer maximumIterations )
{
    // Set up parameters for residual function.
    AtomParameters< Integer, Real, Vector > parameters( departurePosition, 
                                                        departureEpoch, 
                                                        arrivalPosition, 
                                                        timeOfFlight,
                                                        earthGravitationalParameter, 
                                                        earthMeanRadius, 
                                                        referenceTle, 
                                                        absoluteTolerance, 
                                                        relativeTolerance, 
                                                        maximumIterations ); 

    // Set up residual function.
    gsl_multiroot_function atomFunction
        = { &computeAtomResiduals< Integer, Real, Vector >, 
            3, 
            &parameters };

    // Set initial guess.
    gsl_vector* initialGuess = gsl_vector_alloc( 3 );
    for ( Integer i = 0; i < 3; i++ )
    {
        gsl_vector_set( initialGuess, i, departureVelocityGuess[ i ] );      
    }

    // Set up solver type (derivative free).
    const gsl_multiroot_fsolver_type* solverType = gsl_multiroot_fsolver_hybrids;

    // Allocate memory for solver.
    gsl_multiroot_fsolver* solver = gsl_multiroot_fsolver_alloc( solverType, 3 );

    // Set solver to use residual function with initial guess.
    gsl_multiroot_fsolver_set( solver, &atomFunction, initialGuess );

     // Declare current solver status and iteration counter.
    Integer solverStatus = false;
    Integer counter = 0;

    // Set up buffer to store solver status summary table.
    std::ostringstream summary;

    // Print header for summary table to buffer.
    summary << printAtomSolverStateTableHeader( );

    do
    {
        // Print current state of solver for summary table.
        summary << printAtomSolverState( counter, solver );

        // Increment iteration counter.
        ++counter;
        // Execute solver iteration.
        solverStatus = gsl_multiroot_fsolver_iterate( solver );

        // Check if solver is stuck; if it is stuck, break from loop.
        if ( solverStatus )   
        {
            std::cerr << "GSL solver status: " << solverStatus << std::endl;
            std::cerr << summary.str( ) << std::endl;
            std::cerr << std::endl;
            throw std::runtime_error( "ERROR: Non-linear solver is stuck!" );
        }

        // Check if root has been found (within tolerance).
        solverStatus = gsl_multiroot_test_delta( 
          solver->dx, solver->x, absoluteTolerance, relativeTolerance );
    } while ( solverStatus == GSL_CONTINUE && counter < maximumIterations );

    // Save number of iterations.
    numberOfIterations = counter - 1;

    // Print final status of solver to buffer.
    summary << std::endl;
    summary << "Status of non-linear solver: " << gsl_strerror( solverStatus ) << std::endl;
    summary << std::endl;

    // Write buffer contents to solver status summary string.
    solverStatusSummary = summary.str( );

    // Store final departure velocity.
    Vector departureVelocity( 3 );
    for ( Integer i = 0; i < 3; i++ )
    {
        departureVelocity[ i ] = gsl_vector_get( solver->x, i );
    }

    // Set departure state [km/s].
    Vector departureState( 6 );
    for ( Integer i = 0; i < 3; i++ )
    {
        departureState[ i ] = departurePosition[ i ];
    }
    for ( Integer i = 0; i < 3; i++ )
    {
        departureState[ i + 3 ] = departureVelocity[ i ];
    }

    // Convert departure state to TLE.
    std::string dummyString = "";
    Integer dummyInteger = 0;
    const Tle departureTle = convertCartesianStateToTwoLineElements< Integer, Real >(
        departureState, 
        departureEpoch, 
        dummyString, 
        dummyInteger, 
        referenceTle, 
        earthGravitationalParameter, 
        earthMeanRadius, 
        absoluteTolerance, 
        relativeTolerance,
        maximumIterations );

    // Propagate departure TLE by time-of-flight using SGP4 propagator.
    SGP4 sgp4( departureTle );
    Eci arrivalState = sgp4.FindPosition( timeOfFlight );

    Vector arrivalVelocity( 3 );
    arrivalVelocity[ 0 ] = arrivalState.Velocity( ).x;
    arrivalVelocity[ 1 ] = arrivalState.Velocity( ).y;
    arrivalVelocity[ 2 ] = arrivalState.Velocity( ).z;

    // Free up memory.
    gsl_multiroot_fsolver_free( solver );
    gsl_vector_free( initialGuess );

    // Return departure and arrival velocities.
    return std::make_pair< Vector, Vector >( departureVelocity, arrivalVelocity );
}

//! Execute Atom solver.
template< typename Integer, typename Real, typename Vector >
const std::pair< Vector, Vector > executeAtomSolver( 
    const Vector& departurePosition, 
    const DateTime departureEpoch,
    const Vector& arrivalPosition, 
    const Real timeOfFlight, 
    const Vector& departureVelocityGuess )
{
    std::string dummyString = "";
    Integer dummyInteger = 0;
    return executeAtomSolver( departurePosition, 
                              departureEpoch, 
                              arrivalPosition, 
                              timeOfFlight, 
                              departureVelocityGuess,
                              dummyString, 
                              dummyInteger );
}

//! Compute residuals to execute Atom solver.
template< typename Integer, typename Real, typename Vector >
int computeAtomResiduals( const gsl_vector* independentVariables,
                          void* parameters, 
                          gsl_vector* residuals )
{
    // Store parameters locally.
    const Vector departurePosition
        = static_cast< AtomParameters< Integer, Real, Vector >* >( parameters )->departurePosition;

    const DateTime departureEpoch
        = static_cast< AtomParameters< Integer, Real, Vector >* >( parameters )->departureEpoch;

    const Vector targetPosition
        = static_cast< AtomParameters< Integer, Real, Vector >* >( parameters )->targetPosition;

    const Real timeOfFlight
        = static_cast< AtomParameters< Integer, Real, Vector >* >( parameters )->timeOfFlight;

    const Real earthGravitationalParameter
        = static_cast< AtomParameters< Integer, Real, Vector >* >( 
            parameters )->earthGravitationalParameter;

    const Real earthMeanRadius
        = static_cast< AtomParameters< Integer, Real, Vector >* >( parameters )->earthMeanRadius;

    const Tle referenceTle
        = static_cast< AtomParameters< Integer, Real, Vector >* >( parameters )->referenceTle;

    const Real absoluteTolerance
        = static_cast< AtomParameters< Integer, Real, Vector >* >( parameters )->absoluteTolerance;

    const Real relativeTolerance
        = static_cast< AtomParameters< Integer, Real, Vector >* >( parameters )->relativeTolerance;

    const Integer maximumIterations
        = static_cast< AtomParameters< Integer, Real, Vector >* >( parameters )->maximumIterations;

    // Set Departure state [km; km/s].
    Vector departureVelocity( 3 );
    for ( Integer i = 0; i < 3; i++ )
    {
        departureVelocity[ i ] = gsl_vector_get( independentVariables, i );
    }

    Vector departureState( 6 );
    for ( Integer i = 0; i < 3; i++ )
    {
        departureState[ i ] = departurePosition[ i ];
    }
    for ( Integer i = 0; i < 3; i++ )
    {
        departureState[ i + 3 ] = departureVelocity[ i ];
    }

    // Convert departure state to TLE.
    std::string dummyString = "";
    Integer dummyInteger = 0;
    const Tle departureTle = convertCartesianStateToTwoLineElements< Integer, Real >(
        departureState, 
        departureEpoch, 
        dummyString, 
        dummyInteger, 
        referenceTle, 
        earthGravitationalParameter, 
        earthMeanRadius, 
        absoluteTolerance, 
        relativeTolerance,
        maximumIterations );

    // Propagate departure TLE by time-of-flight using SGP4 propagator.
    SGP4 sgp4( departureTle );
    Eci arrivalState = sgp4.FindPosition( timeOfFlight );

    // Evaluate system of non-linear equations and store residuals.    
    gsl_vector_set( residuals, 0,
                    ( arrivalState.Position( ).x - targetPosition[ 0 ] ) / earthMeanRadius );
    gsl_vector_set( residuals, 1, 
                    ( arrivalState.Position( ).y - targetPosition[ 1 ] ) / earthMeanRadius );
    gsl_vector_set( residuals, 2, 
                    ( arrivalState.Position( ).z - targetPosition[ 2 ] ) / earthMeanRadius );

    return GSL_SUCCESS;        
}

//! Parameter struct used by Atom residual function.
template< typename Integer, typename Real, typename Vector >
struct AtomParameters
{ 
public:

    //! Constructor taking parameter values.
    /*!
     * Default constructor, taking parameters to execute Atom solver.
     * @sa executeAtomSolver, computeCartesianToTwoLineElementResiduals
     * @param aDeparturePosition            Cartesian departure position [km]
     * @param aDepartureEpoch               Modified Julian Date (MJD) of departure
     * @param aTargetPosition               Target Cartesian position [km]
     * @param aTimeOfFlight                 Time-of-Flight (TOF) [s]
     * @param anEarthGravitationalParameter Earth gravitational parameter [km^3 s^-2]
     * @param anEarthMeanRadius             Earth mean radius [km]     
     * @param aReferenceTle                 Reference Two-Line-Elements
     * @param anAbsoluteTolerance           Absolute tolerance used to check for convergence
     * @param aRelativeTolerance            Relative tolerance used to check for convergence
     * @param someMaximumIterations         Maximum number of solver iterations permitted
     */
    AtomParameters(
        const Vector aDeparturePosition,
        const DateTime aDepartureEpoch,
        const Vector aTargetPosition,
        const Real aTimeOfFlight,
        const Real anEarthGravitationalParameter,
        const Real anEarthMeanRadius,        
        const Tle aReferenceTle,
        const Real anAbsoluteTolerance,
        const Real aRelativeTolerance,
        const Integer someMaximumIterations )
        : departurePosition( aDeparturePosition ),
          departureEpoch( aDepartureEpoch ),
          targetPosition( aTargetPosition ),
          timeOfFlight( aTimeOfFlight ),
          earthGravitationalParameter( anEarthGravitationalParameter ),
          earthMeanRadius( anEarthMeanRadius ),          
          referenceTle( aReferenceTle ),
          absoluteTolerance( anAbsoluteTolerance ),
          relativeTolerance( aRelativeTolerance ),
          maximumIterations( someMaximumIterations )
    { }

    //! Departure position in Cartesian elements [km].
    const Vector departurePosition;

    //! Departure epoch in Modified Julian Date (MJD).
    const DateTime departureEpoch;

    //! Target position in Cartesian elements [km].
    const Vector targetPosition;

    //! Time-of-Flight (TOF) [s].
    const Real timeOfFlight;

    //! Earth gravitational parameter [km^3 s^-2].
    const Real earthGravitationalParameter;

    //! Earth mean radius [km].
    const Real earthMeanRadius;    

    //! Reference TLE.
    const Tle referenceTle;

    //! Absolute tolerance [-].
    const Real absoluteTolerance;

    //! Relative tolerance [-].
    const Real relativeTolerance;

    //! Maximum number of iterations.
    const Integer maximumIterations;

protected:

private:
};

} // namespace atom

#endif // ATOM_SOLVER_H
