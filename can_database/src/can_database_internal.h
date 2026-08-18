#pragma once
#include "can_database/can_frame.h"

/**
 * @file can_database_internal.h
 * @brief Private, src/-only declarations shared between can_database.c
 *        and generated can_publish.c. Not part of the public API —
 *        not installed under include/can_database/.
 */

void CanDb_Send(CanBusId_t bus, const CanFrame_t *frame);
