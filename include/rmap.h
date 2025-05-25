#pragma once
#include "vars.h"

#define RMAP_NR_DUTY_CYCLE_REMAP_TABLE_SIZE_U16 ((tU16) 1001)  ///< size of the duty cycle remap table

typedef enum
{
    RMAP_REMAP_TABLE_0 = 0,
    RMAP_REMAP_TABLE_1,
    RMAP_REMAP_TABLE_2,
    RMAP_REMAP_TABLE_3,
    RMAP_REMAP_TABLE_4,
    RMAP_REMAP_TABLE_5,
    RMAP_REMAP_TABLE_6,
    RMAP_REMAP_TABLE_7,
    RMAP_REMAP_TABLE_8,
    RMAP_REMAP_TABLE_9,
    RMAP_NR_DUTY_CYCLE_REMAP_TABLES_U8,
    RMAP_USE_1_TO_1 = 0xFF,  ///< use 1 to 1 mapping, i.e., no remapping
} rmap_x_dutyCycleRemapTable_E;

/**
 * @brief Performs a lookup operation in a specified table using the given index.
 *
 * This function retrieves a value from a lookup table based on the provided
 * table identifier and index. It is commonly used for mapping input values
 * to corresponding output values in a predefined table.
 *
 * @param table The identifier of the lookup table to be used.
 *              This parameter specifies which table to perform the lookup in.
 *              Use RMAP_USE_1_TO_1 to instead use a direct 1-1 mapping without remapping.
 * @param index The index within the table to retrieve the value from.
 *              This parameter determines the position in the table to access.
 *              This parameter also functions as the duty cycle in 0.1% [0... 1000] resolution,
 *
 * @return The value retrieved from the lookup table at the specified index.
 *         The return type is `tU16`, which represents a 16-bit unsigned value.
 */
tU16 rmap_lookup(tU16 table, tU16 index);