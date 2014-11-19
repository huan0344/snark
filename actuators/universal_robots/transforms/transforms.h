#ifndef SNARK_ACTUATORS_UR_ROBOTIC_ARM_UR5_TRANSFORMS_H
#define SNARK_ACTUATORS_UR_ROBOTIC_ARM_UR5_TRANSFORMS_H

#include <boost/array.hpp>
#include <snark/math/applications/frame.h>
#include "../units.h"
extern "C" {
#include "simulink/tcp_transform.h"
}

/* External inputs (root inport signals with auto storage) */
extern ExtU_tcp_transform_T tcp_transform_U;

/* External outputs (root outports fed by signals with auto storage) */
extern ExtY_tcp_transform_T tcp_transform_Y;

/* Model entry point functions */
extern void tcp_transform_initialize(void);
extern void tcp_transform_step(void);
extern void tcp_transform_terminate(void);


namespace snark { namespace ur { namespace ur5 {
    
/// Given the joint angles for the UR5, give the position of the TCP and of mounted laser ( 11mm above the TCP )
void tcp_transform( const boost::array< plane_angle_t, 6 >& joint_angles, 
                    snark::applications::position& position,
                    snark::applications::position& laser );

} } } // namespace snark { namespace ur { namespace ur5 {

#endif // SNARK_ACTUATORS_UR_ROBOTIC_ARM_UR5_TRANSFORMS_H