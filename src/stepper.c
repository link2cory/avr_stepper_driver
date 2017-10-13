#include "stepper.h"

/*******************************************************************************
* Private Defines
*******************************************************************************/
#define MAX_STEPPERS 2
#define MAX_STEPPER_POS 199

/*******************************************************************************
* Private Typedefs
*******************************************************************************/
typedef enum stepper_status_t {
  STEPPER_STATUS_INACTIVE,
  STEPPER_STATUS_ACTIVE
} stepper_status_t;

typedef struct stepper_t {
  uint8_t *dir_port;
  uint8_t *dir_port_ddr;
  uint8_t dir_pin;

  uint8_t *enable_port;
  uint8_t *enable_port_ddr;
  uint8_t enable_pin;

  uint8_t *step_port;
  uint8_t *step_port_ddr;
  uint8_t step_pin;

  uint8_t *ms1_port;
  uint8_t *ms1_port_ddr;
  uint8_t ms1_pin;

  uint8_t *ms2_port;
  uint8_t *ms2_port_ddr;
  uint8_t ms2_pin;

  uint8_t *ms3_port;
  uint8_t *ms3_port_ddr;
  uint8_t ms3_pin;

  stepper_status_t status;
  uint8_t speed;
  stepper_step_size_t step_size;
  uint8_t desired_pos;
  stepper_dir_t dir;
} stepper_t;
/*******************************************************************************
* Private Data
*******************************************************************************/
static stepper_t steppers[MAX_STEPPERS];
/*******************************************************************************
* Public Function Definitions
*******************************************************************************/
stepper_err_t stepper_construct(
  stepper_attr_t config,
  stepper_descriptor_t *handle
) {
  uint8_t i;
  stepper_err_t err = STEPPER_ERR_NONE_AVAILABLE;

  for (i=0;i<MAX_STEPPERS;i++) {
    if (steppers[i].status == STEPPER_STATUS_INACTIVE) {
      steppers[i].dir_port = config.dir_port;
      steppers[i].dir_port_ddr = config.dir_port_ddr;
      steppers[i].dir_pin = config.dir_pin;

      steppers[i].enable_port = config.enable_port;
      steppers[i].enable_port_ddr = config.enable_port_ddr;
      steppers[i].enable_pin = config.enable_pin;

      steppers[i].step_port = config.step_port;
      steppers[i].step_port_ddr = config.step_port_ddr;
      steppers[i].step_pin = config.step_pin;

      steppers[i].ms1_port = config.ms1_port;
      steppers[i].ms1_port_ddr = config.ms1_port_ddr;
      steppers[i].ms1_pin = config.ms1_pin;

      steppers[i].ms2_port = config.ms2_port;
      steppers[i].ms2_port_ddr = config.ms2_port_ddr;
      steppers[i].ms2_pin = config.ms2_pin;

      steppers[i].ms3_port = config.ms3_port;
      steppers[i].ms3_port_ddr = config.ms3_port_ddr;
      steppers[i].ms3_pin = config.ms3_pin;

      *steppers[i].dir_port = 0;
      *steppers[i].dir_port_ddr |= steppers[i].dir_pin;

      *steppers[i].enable_port = 0;
      *steppers[i].enable_port_ddr |= steppers[i].enable_pin;

      *steppers[i].step_port = 0;
      *steppers[i].step_port_ddr |= steppers[i].step_pin;

      *steppers[i].ms1_port = 0;
      *steppers[i].ms1_port_ddr |= steppers[i].ms1_pin;

      *steppers[i].ms2_port = 0;
      *steppers[i].ms2_port_ddr |= steppers[i].ms2_pin;

      *steppers[i].ms3_port = 0;
      *steppers[i].ms3_port_ddr |= steppers[i].ms3_pin;

      steppers[i].status = STEPPER_STATUS_ACTIVE;
      steppers[i].speed = config.speed;

      *handle = i;

      err = STEPPER_ERR_NONE;
      break;
    }
  }

  return err;
}

void stepper_destruct(stepper_descriptor_t handle) {
  steppers[handle].status = STEPPER_STATUS_INACTIVE;

}

stepper_err_t stepper_setSpeed(stepper_descriptor_t handle, uint8_t speed) {
  stepper_err_t err = STEPPER_ERR_NONE;

  if (handle >= MAX_STEPPERS
    || steppers[handle].status != STEPPER_STATUS_ACTIVE
  ) {
    err = STEPPER_ERR_HANDLE_INVALID;
  } else {
    steppers[handle].speed = speed;
  }
  return err;
}

uint8_t stepper_getSpeed(stepper_descriptor_t handle) {
  return steppers[handle].speed;
}

stepper_err_t stepper_setStepSize(
  stepper_descriptor_t handle,
  stepper_step_size_t step_size
) {
  stepper_err_t err = STEPPER_ERR_NONE;

  if (handle >= MAX_STEPPERS
    || steppers[handle].status != STEPPER_STATUS_ACTIVE
  ) {
    err = STEPPER_ERR_HANDLE_INVALID;
  } else {
    // set ms1
    if (step_size == STEPPER_STEP_SIZE_HALF
      || step_size == STEPPER_STEP_SIZE_EIGHTH
      || step_size == STEPPER_STEP_SIZE_SIXTEENTH
    ) {
      *steppers[handle].ms1_port |= steppers[handle].ms1_pin;
    } else {
      *steppers[handle].ms1_port &= ~steppers[handle].ms1_pin;
    }

    // set ms2
    if (step_size == STEPPER_STEP_SIZE_QUARTER
      || step_size == STEPPER_STEP_SIZE_EIGHTH
      || step_size == STEPPER_STEP_SIZE_SIXTEENTH
    ) {
      *steppers[handle].ms2_port |= steppers[handle].ms2_pin;
    } else {
      *steppers[handle].ms2_port &= ~steppers[handle].ms2_pin;
    }

    // set ms3
    if (step_size == STEPPER_STEP_SIZE_SIXTEENTH
    ) {
      *steppers[handle].ms3_port |= steppers[handle].ms3_pin;
    } else {
      *steppers[handle].ms3_port &= ~steppers[handle].ms3_pin;
    }

    steppers[handle].step_size = step_size;
  }
  return err;
}

stepper_step_size_t stepper_getStepSize(stepper_descriptor_t handle) {
  return steppers[handle].step_size;
}

stepper_err_t stepper_setPos(stepper_descriptor_t handle, uint8_t pos) {
  stepper_err_t err = STEPPER_ERR_NONE;

  if (handle >= MAX_STEPPERS
    || steppers[handle].status != STEPPER_STATUS_ACTIVE
  ) {
    err = STEPPER_ERR_HANDLE_INVALID;
  } else {
    if (pos <= MAX_STEPPER_POS) {
      steppers[handle].desired_pos = pos;
    } else {
      err = STEPPER_ERR_POSITION_INVALID;
    }
  }
  return err;
}

uint8_t stepper_getDesiredPos(stepper_descriptor_t handle) {
  return steppers[handle].desired_pos;
}

stepper_err_t stepper_setDir(stepper_descriptor_t handle, stepper_dir_t dir) {
  stepper_err_t err = STEPPER_ERR_NONE;

  if (handle >= MAX_STEPPERS
    || steppers[handle].status != STEPPER_STATUS_ACTIVE
  ) {
    err = STEPPER_ERR_HANDLE_INVALID;
  } else {
    steppers[handle].dir = dir;
    *steppers[handle].dir_port |= (dir << steppers[handle].dir_pin);
  }
  return err;
}

stepper_dir_t stepper_getDir(stepper_descriptor_t handle) {
  return steppers[handle].dir;
}

stepper_err_t stepper_stepEngage(stepper_descriptor_t handle) {
  stepper_err_t err = STEPPER_ERR_NONE;

  if (handle >= MAX_STEPPERS
    || steppers[handle].status != STEPPER_STATUS_ACTIVE
  ) {
    err = STEPPER_ERR_HANDLE_INVALID;
  } else {
    *steppers[handle].step_port |= steppers[handle].step_pin;
  }

  return err;
}
