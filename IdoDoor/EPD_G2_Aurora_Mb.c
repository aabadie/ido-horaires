/**
* \file
*
* \brief The waveform driving processes and updating stages of G2 COG with Vizplex or Aurora_Mb EPD
*
* Copyright (c) 2012-2015 Pervasive Displays Inc. All rights reserved.
*
*  Authors: Pervasive Displays Inc.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*  1. Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*  2. Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in
*     the documentation and/or other materials provided with the
*     distribution.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "COG_FPL.h"
#include "epd_board.h"

#ifdef G2_Aurora_Mb
#define ADDRESS_NULL		0xffffffff

void dummy_line( void );
void nothing_frame ( void );
void nothing_frame_for_19_26 ( void );
/**
 * \brief The COG parameters of different EPD size
 */
const struct COG_parameters_t COG_parameters[COUNT_OF_EPD_TYPE] =
{
    {
        // FOR 1.44"
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xFF, 0x00},
        0x03,
        ( 128 / 8 ),
        96,
        ( ( ( ( 128 + 96 ) * 2 ) / 8 ) + 1 ),
        0,
        480
    },
    {
        // For 2.0"
        {0x00, 0x00, 0x00, 0x00, 0x01, 0xFF, 0xE0, 0x00},
        0x03,
        ( 200 / 8 ),
        96,
        ( ( ( ( 200 + 96 ) * 2 ) / 8 ) + 1 ),
        0,
        480
    },
    {
        // For 2.7"
        {0x00, 0x00, 0x00, 0x7F, 0xFF, 0xFE, 0x00, 0x00},
        0x00,
        ( 264 / 8 ),
        176,
        ( ( ( ( 264 + 176 ) * 2 ) / 8 ) + 1 ),
        0,
        630
    },
    {
        // For 1.9"
        {0x00, 0x00, 0x00, 0x03, 0xFC, 0x00, 0x00, 0xFF},
        0x03,
        ( 144 / 8 ),
        128,
        ( ( ( ( 144 + 128 ) * 2 ) / 8 ) + 1 ),
        0,
        480
    },
    {
        // For 2.6"
        {0x00, 0x00, 0x1F, 0xE0, 0x00, 0x00, 0x00, 0xFF},
        0x03,
        ( 232 / 8 ),
        128,
        ( ( ( ( 232 + 128 ) * 2 ) / 8 ) + 1 ),
        0,
        630
    }
};

/* Temperature factor combines with stage time for each driving stage */
const uint16_t temperature_table[COUNT_OF_EPD_TYPE][8] =
{
    {( 480 * 17 ), ( 480 * 12 ), ( 480 * 8 ), ( 480 * 4 ), ( 480 * 3 ), ( 480 * 2 ), ( 480 * 1 ), ( 480 * 0.7 )}, // 1.44"
    {( 480 * 17 ), ( 480 * 12 ), ( 480 * 8 ), ( 480 * 4 ), ( 480 * 3 ), ( 480 * 2 ), ( 480 * 1 ), ( 480 * 0.7 )}, // 2.0"
    {( 630 * 17 ), ( 630 * 12 ), ( 630 * 8 ), ( 630 * 4 ), ( 630 * 3 ), ( 630 * 2 ), ( 630 * 1 ), ( 630 * 0.7 )}, // 2.7"
    {( 480 * 17 ), ( 480 * 12 ), ( 480 * 8 ), ( 480 * 4 ), ( 480 * 3 ), ( 480 * 2 ), ( 480 * 1 ), ( 480 * 0.7 )}, // 1.9"
    {( 630 * 17 ), ( 630 * 12 ), ( 630 * 8 ), ( 630 * 4 ), ( 630 * 3 ), ( 630 * 2 ), ( 630 * 1 ), ( 630 * 0.7 )}, // 2.6"
};

/* Lookup table for 1.9" and 2.6" */
const uint8_t data_lookuptable[4][16] =
{
    {0xAA, 0xEA, 0xBA, 0xFA, 0xAE, 0xEE, 0xBE, 0xFE, 0xAB, 0xEB, 0xBB, 0xFB, 0xAF, 0xEF, 0xBF, 0xFF},
    {0x00, 0x80, 0x20, 0xA0, 0x08, 0x88, 0x28, 0xA8, 0x02, 0x82, 0x22, 0xA2, 0x0A, 0x8A, 0x2A, 0xAA},
    {0x55, 0xD5, 0x75, 0xF5, 0x5D, 0xDD, 0x7D, 0xFD, 0x57, 0xD7, 0x77, 0xF7, 0x5F, 0xDF, 0x7F, 0xFF},
    {0xFF, 0xBF, 0xEF, 0xAF, 0xFB, 0xBB, 0xEB, 0xAB, 0xFE, 0xBE, 0xEE, 0xAE, 0xFA, 0xBA, 0xEA, 0xAA}
};
const uint8_t scan_lookuptable[8] = {0x03, 0xc0, 0x0c, 0x30, 0x30, 0x0c, 0xc0, 0x03}; // 1.9",2.6" odd scan
const uint8_t BORDER_TABLE[4] = {0x00, 0x00, 0x00, 0xAA};
const uint8_t SCAN_TABLE[4] = {0xC0, 0x30, 0x0C, 0x03};
static uint16_t stage_time;
static COG_line_data_packet_type COG_Line;
static EPD_read_memory_handler _On_EPD_read_flash;
static uint16_t current_frame_time;
static uint8_t  *data_line_even;
static uint8_t  *data_line_odd;
static uint8_t  *data_line_scan;
static uint8_t  *data_line_border_byte;
static uint8_t cur_EPD_type_index = 0;
uint8_t *previous_lin, *new_line;
uint16_t PartialUpdate_StageTime = D_PartialUpdate_StageTime;

/**
 * \brief According to EPD size and temperature to get stage_time
 * \note Refer to COG G2 document Section 5.3 for more details
 */
static void set_temperature_factor( int8_t temperature )
{
    if ( temperature <= -10 )
    {
        stage_time = temperature_table[cur_EPD_type_index][0];
    }
    else if ( -5 >= temperature && temperature > -10 )
    {
        stage_time = temperature_table[cur_EPD_type_index][1];
    }
    else if ( 5 >= temperature && temperature > -5 )
    {
        stage_time = temperature_table[cur_EPD_type_index][2];
    }
    else if ( 10 >= temperature && temperature > 5 )
    {
        stage_time = temperature_table[cur_EPD_type_index][3];
    }
    else if ( 15 >= temperature && temperature > 10 )
    {
        stage_time = temperature_table[cur_EPD_type_index][4];
    }
    else if ( 20 >= temperature && temperature > 15 )
    {
        stage_time = temperature_table[cur_EPD_type_index][5];
    }
    else if ( 40 >= temperature && temperature > 20 )
    {
        stage_time = temperature_table[cur_EPD_type_index][6];
    }
    else
    {
        stage_time = temperature_table[cur_EPD_type_index][7];
    }
}


/**
 * \brief Select the EPD size to get line data array for driving COG
 *
 */
void COG_driver_EPDtype_select( void )
{
    switch( cur_EPD_type_index )
    {
        case EPD_144:
            data_line_even = &COG_Line.line_data_by_size.line_data_for_144.even[0];
            data_line_odd  = &COG_Line.line_data_by_size.line_data_for_144.odd[0];
            data_line_scan = &COG_Line.line_data_by_size.line_data_for_144.scan[0];
            data_line_border_byte = &COG_Line.line_data_by_size.line_data_for_144.border_byte;
            break;
        case EPD_200:
            data_line_even = &COG_Line.line_data_by_size.line_data_for_200.even[0];
            data_line_odd  = &COG_Line.line_data_by_size.line_data_for_200.odd[0];
            data_line_scan = &COG_Line.line_data_by_size.line_data_for_200.scan[0];
            data_line_border_byte = &COG_Line.line_data_by_size.line_data_for_200.dummy_data;
            break;
        case EPD_270:
            data_line_even = &COG_Line.line_data_by_size.line_data_for_270.even[0];
            data_line_odd  = &COG_Line.line_data_by_size.line_data_for_270.odd[0];
            data_line_scan = &COG_Line.line_data_by_size.line_data_for_270.scan[0];
            data_line_border_byte = &COG_Line.line_data_by_size.line_data_for_270.dummy_data;
            break;
        case EPD_190:
            data_line_even = &COG_Line.line_data_by_size.line_data_for_190.scan_even[0];
            data_line_odd  = &COG_Line.line_data_by_size.line_data_for_190.scan_odd[0];
            data_line_scan = &COG_Line.line_data_by_size.line_data_for_190.data[0];
            data_line_border_byte = &COG_Line.line_data_by_size.line_data_for_190.border_byte;
            break;
        case EPD_260:
            data_line_even = &COG_Line.line_data_by_size.line_data_for_260.scan_even[0];
            data_line_odd  = &COG_Line.line_data_by_size.line_data_for_260.scan_odd[0];
            data_line_scan = &COG_Line.line_data_by_size.line_data_for_260.data[0];
            data_line_border_byte = &COG_Line.line_data_by_size.line_data_for_260.border_byte;
            break;
    }
    *data_line_border_byte = 0x00;
}


/**
* \brief Power on COG Driver
* \note For detailed flow and description, please refer to the COG G2 document Section 3.
*/
void EPD_power_on ( uint8_t EPD_type_index, int8_t temperature )
{

    cur_EPD_type_index = EPD_type_index;
    /* Initial state */
    EPD_Vcc_turn_on();

    EPD_cs_high();
    epd_spi_init();
    EPD_cs_high();
    EPD_border_high();
    EPD_rst_high();
    DelayMs( 6 );
    EPD_rst_low();
    DelayMs( 6 );
    EPD_rst_high();
    DelayMs( 6 );
    // Sense temperature to determine Temperature Factor
    set_temperature_factor( temperature );
}


/**
 * \brief Initialize COG Driver
 * \note For detailed flow and description, please refer to the COG G2 document Section 4.
 */
uint8_t EPD_initialize_driver ( void )
{
    uint16_t i;
    // Empty the Line buffer
    for ( i = 0; i < LINE_BUFFER_DATA_SIZE; i ++ )
    {
        COG_Line.uint8[i] = 0x00;
    }
    // Determine the EPD size for driving COG
    COG_driver_EPDtype_select();

    i = 0;

    while ( EPD_IsBusy() )
    {
        if( ( i++ ) >= 0x0FFF )
        {
            return ERROR_BUSY;
        }
    }

    //Check COG ID
    if( ( SPI_R( 0x72, 0x00 ) & 0x0f ) != 0x02 )
    {
        return ERROR_COG_ID;
    }

    //Disable OE
    epd_spi_send_byte( 0x02, 0x40 );

    //Check Breakage
    if( ( SPI_R( 0x0F, 0x00 ) & 0x80 ) != 0x80 )
    {
        return ERROR_BREAKAGE;
    }

    //Power Saving Mode
    epd_spi_send_byte( 0x0B, 0x02 );

    //Channel Select
    epd_spi_send ( 0x01, ( uint8_t * )&COG_parameters[cur_EPD_type_index].channel_select, 8 );

    //High Power Mode Osc Setting
    epd_spi_send_byte( 0x07, 0xD1 );

    //Power Setting
    epd_spi_send_byte( 0x08, 0x02 );

    //Set Vcom level
    epd_spi_send_byte( 0x09, 0xC2 );

    //Power Setting
    epd_spi_send_byte( 0x04, 0x03 );

    //Driver latch on
    epd_spi_send_byte( 0x03, 0x01 );

    //Driver latch off
    epd_spi_send_byte( 0x03, 0x00 );

    DelayMs( 5 );

    //Chargepump Start
    i = 0;
    do
    {
        //Start chargepump positive V
        //VGH & VDH on
        epd_spi_send_byte( 0x05, 0x01 );

        DelayMs( 240 );

        //Start chargepump neg voltage
        //VGL & VDL on
        epd_spi_send_byte( 0x05, 0x03 );

        DelayMs( 40 );

        //Set chargepump
        //Vcom_Driver to ON
        //Vcom_Driver on
        epd_spi_send_byte( 0x05, 0x0F );

        DelayMs( 40 );

        //Check DC/DC
        if( ( SPI_R( 0x0F, 0x00 ) & 0x40 ) != 0x00 )
        {
            //Output enable to disable
            epd_spi_send_byte( 0x02, 0x40 );
            break;
        }

    }
    while( ( i++ ) != 4 );

    if( i >= 4 )
    {
        return ERROR_CHARGEPUMP;
    }
    else
    {
        return RES_OK;
    }
}



/**
 * \brief The Global Update driving stages for getting Odd/Even data and writing the data
 * from memory array to COG
 *
 * \note
 * - One dot/pixel is comprised of 2 bits which are White(10), Black(11) or Nothing(01 or 00).
 *   The image data bytes must be divided into Odd and Even bytes.
 * - For 1.44", 2" and 2.7", the line data flow is half_Data-Scan-half_Data
 * - For more details on the driving stages, please refer to the COG G2 document Section 5.
 *
 * \param image_ptr The pointer of memory that stores image that will send to COG
 * \param stage_no The assigned stage number that will proceed
 */
static void stage_handle_array( uint8_t const *image_prt, long image_data_address, uint8_t stage_no )
{
    /* x for horizontal_size loop, y for vertical_size loop, which are EPD pixel size */
    uint16_t x, y, k;
    int16_t scanline_no = 0;
    uint8_t	temp_byte; // Temporary storage for image data check
    uint8_t const *backup_image_prt; // Backup image address pointer
    long original_image_address;
    uint8_t byte_array[COG_line_Max_Size];
    original_image_address = image_data_address;
    backup_image_prt = image_prt;
    current_frame_time = 0;
    /* Start a system SysTick timer to ensure the same duration of each stage  */
    start_EPD_timer();

    /* Do while total time of frames exceed stage time
    * Per frame */
    do
    {
        image_prt = backup_image_prt;
        image_data_address = original_image_address;
        /* Per data line (vertical size) */
        for ( y = 0; y < COG_parameters[cur_EPD_type_index].vertical_size; y++ )
        {
            k = COG_parameters[cur_EPD_type_index].horizontal_size - 1;
            if( _On_EPD_read_flash != NULL )
            {
                _On_EPD_read_flash( image_data_address, ( uint8_t * )&byte_array,
                                    COG_parameters[cur_EPD_type_index].horizontal_size );
                image_prt = &byte_array[0];
            }
            for ( x = 0; x < COG_parameters[cur_EPD_type_index].horizontal_size; x++ )
            {
                temp_byte = *image_prt++;
                switch( stage_no )
                {
                    case Stage1: // Compensate, Inverse previous image
                        data_line_odd[x]     = ( ( temp_byte & 0x40 ) ? BLACK3  : WHITE3 );
                        data_line_odd[x]    |= ( ( temp_byte & 0x10 ) ? BLACK2  : WHITE2 );
                        data_line_odd[x]    |= ( ( temp_byte & 0x04 ) ? BLACK1  : WHITE1 );
                        data_line_odd[x]    |= ( ( temp_byte & 0x01 ) ? BLACK0  : WHITE0 );

                        data_line_even[k]    = ( ( temp_byte & 0x80 ) ? BLACK0  : WHITE0 );
                        data_line_even[k]   |= ( ( temp_byte & 0x20 ) ? BLACK1  : WHITE1 );
                        data_line_even[k]   |= ( ( temp_byte & 0x08 ) ? BLACK2  : WHITE2 );
                        data_line_even[k--] |= ( ( temp_byte & 0x02 ) ? BLACK3  : WHITE3 );
                        break;
                    case Stage2: // White
                        data_line_odd[x]     = ( ( temp_byte & 0x40 ) ?  WHITE3 : NOTHING3 );
                        data_line_odd[x]    |= ( ( temp_byte & 0x10 ) ?  WHITE2 : NOTHING2 );
                        data_line_odd[x]    |= ( ( temp_byte & 0x04 ) ?  WHITE1 : NOTHING1 );
                        data_line_odd[x]    |= ( ( temp_byte & 0x01 ) ?  WHITE0 : NOTHING0 );

                        data_line_even[k]    = ( ( temp_byte & 0x80 ) ?  WHITE0 : NOTHING0 );
                        data_line_even[k]   |= ( ( temp_byte & 0x20 ) ?  WHITE1 : NOTHING1 );
                        data_line_even[k]   |= ( ( temp_byte & 0x08 ) ?  WHITE2 : NOTHING2 );
                        data_line_even[k--] |= ( ( temp_byte & 0x02 ) ?  WHITE3 : NOTHING3 );
                        break;
                    case Stage3: // Inverse new image
                        data_line_odd[x]     = ( ( temp_byte & 0x40 ) ? BLACK3  : NOTHING3 );
                        data_line_odd[x]    |= ( ( temp_byte & 0x10 ) ? BLACK2  : NOTHING2 );
                        data_line_odd[x]    |= ( ( temp_byte & 0x04 ) ? BLACK1  : NOTHING1 );
                        data_line_odd[x]    |= ( ( temp_byte & 0x01 ) ? BLACK0  : NOTHING0 );

                        data_line_even[k]    = ( ( temp_byte & 0x80 ) ? BLACK0  : NOTHING0 );
                        data_line_even[k]   |= ( ( temp_byte & 0x20 ) ? BLACK1  : NOTHING1 );
                        data_line_even[k]   |= ( ( temp_byte & 0x08 ) ? BLACK2  : NOTHING2 );
                        data_line_even[k--] |= ( ( temp_byte & 0x02 ) ? BLACK3  : NOTHING3 );
                        break;
                    case Stage4: // New image
                        data_line_odd[x]     = ( ( temp_byte & 0x40 ) ? WHITE3  : BLACK3 );
                        data_line_odd[x]    |= ( ( temp_byte & 0x10 ) ? WHITE2  : BLACK2 );
                        data_line_odd[x]    |= ( ( temp_byte & 0x04 ) ? WHITE1  : BLACK1 );
                        data_line_odd[x]    |= ( ( temp_byte & 0x01 ) ? WHITE0  : BLACK0 );

                        data_line_even[k]    = ( ( temp_byte & 0x80 ) ? WHITE0  : BLACK0 );
                        data_line_even[k]   |= ( ( temp_byte & 0x20 ) ? WHITE1  : BLACK1 );
                        data_line_even[k]   |= ( ( temp_byte & 0x08 ) ? WHITE2  : BLACK2 );
                        data_line_even[k--] |= ( ( temp_byte & 0x02 ) ? WHITE3  : BLACK3 );
                        break;
                }

            }

            image_data_address += Line_Offset( cur_EPD_type_index );

            scanline_no = ( COG_parameters[cur_EPD_type_index].vertical_size - 1 ) - y;

            /* Scan byte shift per data line */
            data_line_scan[( scanline_no >> 2 )] = SCAN_TABLE[( scanline_no % 4 )];

            /* Sending data */
            epd_spi_send ( 0x0A, ( uint8_t * )&COG_Line.uint8,
                           COG_parameters[cur_EPD_type_index].data_line_size );

            /* Turn on Output Enable */
            epd_spi_send_byte ( 0x02, 0x07 );

            data_line_scan[( scanline_no >> 2 )] = 0;
        }
        /* Count the frame time with offset */
        current_frame_time = ( uint16_t )get_EPD_timer_value();

        board_reset_watchdog_counter();
    }
    while ( stage_time > current_frame_time ) ;

    /* Do while the SysTick timer fulfills the stage time */
    while( stage_time > get_EPD_timer_value() )
    {
        board_reset_watchdog_counter();
    }

    /* Stop system timer */
    stop_EPD_timer();
}



/**
 * \brief The driving stages for getting Odd/Even data and writing the data
 * from memory array to COG for 1.9" and 2.6"
 * \note For 1.9" and 2.6", the line data flow is half_Scan-Data-half_Scan
 *
 * \param image_ptr The pointer of memory that stores image that will send to COG
 * \param stage_no The assigned stage number that will proceed
 */
static void stage_handle_array_for_19_26( uint8_t const *image_prt, long image_data_address, uint8_t stage_no )
{
    /* x for horizontal_size loop, y for vertical_size loop, which are EPD pixel size */
    uint16_t x, y;
    uint8_t cnt;
    uint8_t s_index, b_index, line_len;
    long original_image_address; // Backup original image address
    uint8_t	temp_byte; // Temporary storage for image data check
    uint8_t const *backup_image_prt; // Backup image address pointer
    uint8_t byte_array[COG_line_Max_Size];
    backup_image_prt = image_prt;
    original_image_address = image_data_address;
    current_frame_time = COG_parameters[cur_EPD_type_index].frame_time_offset;
    /* Start a system SysTick timer to ensure the same duration of each stage  */
    start_EPD_timer();
    line_len = ( COG_parameters[cur_EPD_type_index].vertical_size / 8 ) - 1;
    /* Do while total time of frames exceed stage time
    * Per frame */
    do
    {
        image_prt = backup_image_prt;
        image_data_address = original_image_address;
        *data_line_border_byte = BORDER_TABLE[stage_no];
        /* Per data line (vertical size) */
        for ( y = 0; y < COG_parameters[cur_EPD_type_index].vertical_size; y++ )
        {
            if( _On_EPD_read_flash != NULL )
            {
                _On_EPD_read_flash( image_data_address, ( uint8_t * )&byte_array,
                                    COG_parameters[cur_EPD_type_index].horizontal_size );
                image_prt = &byte_array[0];
            }

            cnt = ( COG_parameters[cur_EPD_type_index].horizontal_size * 2 ) - 1;
            for ( x = 0; x < COG_parameters[cur_EPD_type_index].horizontal_size; x++ )
            {
                temp_byte = *image_prt++;
                data_line_scan[cnt--] = data_lookuptable[stage_no][( ( temp_byte >> 4 ) & 0x0f )];
                data_line_scan[( cnt-- )] = data_lookuptable[stage_no][( temp_byte & 0x0f )];
            }
            s_index = y >> 3;
            b_index = ( y % 8 );
            if( ( y % 2 ) == 0 )
            {
                //odd
                data_line_even[line_len - s_index] = scan_lookuptable[b_index];
            }
            else
            {
                //even
                data_line_odd[s_index] = scan_lookuptable[b_index];
            }
            //image_data_address+=COG_parameters[cur_EPD_type_index].horizontal_size;
            image_data_address += Line_Offset( cur_EPD_type_index );
            /* Sending data */
            epd_spi_send ( 0x0A, ( uint8_t * )&COG_Line.uint8,
                           COG_parameters[cur_EPD_type_index].data_line_size );

            /* Turn on Output Enable */
            epd_spi_send_byte ( 0x02, 0x07 );

            data_line_odd[s_index] = 0;
            data_line_even[line_len - s_index] = 0;
        }
        /* Count the frame time with offset */
        current_frame_time = ( uint16_t )get_EPD_timer_value() +
                             COG_parameters[cur_EPD_type_index].frame_time_offset;
    }
    while ( stage_time > current_frame_time );

    /* Do while the SysTick timer fulfills the stage time */
    while( stage_time > get_EPD_timer_value() );

    /* Stop system timer */
    stop_EPD_timer();
}

/**
 * \brief Write image data from memory array to the EPD
 * \note
 * - There are 4 stages to complete an image update on EPD.
 * - The same stage time for Each of the 4 stages.
 * - For more detail on driving stages, please refer to COG G2 document Section 5.
 *
 * \param previous_image_ptr The pointer of memory that stores previous image
 * \param new_image_ptr The pointer of memory that stores new image
 */
void EPD_display_from_array_prt ( uint8_t const *previous_image_ptr, uint8_t const *new_image_ptr )
{
    _On_EPD_read_flash = NULL;
    if( cur_EPD_type_index == EPD_190 || cur_EPD_type_index == EPD_260 )
    {
        stage_handle_array_for_19_26( previous_image_ptr, ADDRESS_NULL, Stage1 );
        stage_handle_array_for_19_26( previous_image_ptr, ADDRESS_NULL, Stage2 );
        stage_handle_array_for_19_26( new_image_ptr, ADDRESS_NULL, Stage3 );
        stage_handle_array_for_19_26( new_image_ptr, ADDRESS_NULL, Stage4 );
    }
    else
    {
        stage_handle_array( previous_image_ptr, ADDRESS_NULL, Stage1 );
        stage_handle_array( previous_image_ptr, ADDRESS_NULL, Stage2 );
        stage_handle_array( new_image_ptr, ADDRESS_NULL, Stage3 );
        stage_handle_array( new_image_ptr, ADDRESS_NULL, Stage4 );
    }
}

/**
 * \brief Write image data from Flash memory to the EPD
 *
 * \param previous_image_flash_address The previous image address of flash memory
 * \param new_image_flash_address The new image address of flash memory
 * \param On_EPD_read_flash Developer needs to create an external function to read flash
 */
void EPD_display_from_flash_prt ( long previous_image_flash_address,
                                  long new_image_flash_address, EPD_read_memory_handler On_EPD_read_flash )
{
    _On_EPD_read_flash = On_EPD_read_flash;
    if( cur_EPD_type_index == EPD_190 || cur_EPD_type_index == EPD_260 )
    {
        stage_handle_array_for_19_26( NULL, previous_image_flash_address, Stage1 );
        stage_handle_array_for_19_26( NULL, previous_image_flash_address, Stage2 );
        stage_handle_array_for_19_26( NULL, new_image_flash_address, Stage3 );
        stage_handle_array_for_19_26( NULL, new_image_flash_address, Stage4 );
    }
    else
    {
        stage_handle_array( NULL, previous_image_flash_address, Stage1 );
        stage_handle_array( NULL, previous_image_flash_address, Stage2 );
        stage_handle_array( NULL, new_image_flash_address, Stage3 );
        stage_handle_array( NULL, new_image_flash_address, Stage4 );
    }
}

/**
 * \brief The Partial Update driving stages for getting Odd/Even data and writing the data
 * from memory array to COG
 *
 * \note
 * - This function for 1.9" and 2.6"
 * - Just run the last stage (of 4 stages of global update) to update image immediately.
 * - If the new data byte is same as previous data byte, send ��Nothing�� data byte which
 *   means the data byte on EPD won��t be changed.
 * �V If the new data byte is different from the previous data byte, send the new data byte.
 * - For more details on the Partial Update driving method, please contact techsupport@pervasivedisplays.com *
 */
static void stage_handle_partial_update_for_19_26( uint8_t const *previous_image_ptr, uint8_t const *new_image_ptr,
        long previous_image_address, long new_image_address )
{
    /* x for horizontal_size loop, y for vertical_size loop, which are EPD pixel size */
    uint16_t x, y;
    uint8_t cnt;
    uint8_t s_index, b_index, line_len;
    uint8_t const *r_previous_image_ptr, *r_new_image_ptr;
    uint8_t old_byte, new_byte;
    long address_offset = 0;
    start_EPD_timer();
    line_len = ( COG_parameters[cur_EPD_type_index].vertical_size / 8 ) - 1;
    /* Do while total time of frames exceed stage time
    * Per frame */
    do
    {
        address_offset = 0;
        r_previous_image_ptr = previous_image_ptr;
        r_new_image_ptr = new_image_ptr;

        *data_line_border_byte = BORDER_BYTE_W;
        /* Per data line (vertical size) */
        for ( y = 0; y < COG_parameters[cur_EPD_type_index].vertical_size; y++ )
        {
            if( _On_EPD_read_flash != NULL )
            {
                _On_EPD_read_flash( ( previous_image_address + address_offset ),
                                    previous_lin,
                                    COG_parameters[cur_EPD_type_index].horizontal_size );
                r_previous_image_ptr = previous_lin;

                _On_EPD_read_flash( ( new_image_address + address_offset ),
                                    new_line,
                                    COG_parameters[cur_EPD_type_index].horizontal_size );
                r_new_image_ptr = new_line;

            }
            cnt = ( COG_parameters[cur_EPD_type_index].horizontal_size * 2 ) - 1;
            for ( x = 0; x < COG_parameters[cur_EPD_type_index].horizontal_size; x++ )
            {
                old_byte = *r_previous_image_ptr++;
                new_byte = *r_new_image_ptr++;

                data_line_scan[cnt]  	 = ( ( old_byte ^ new_byte ) & 0x80 ) ? ( ( new_byte & 0x80 ) ? WHITE0 : BLACK0 ) : NOTHING0;
                data_line_scan[cnt] 	    |= ( ( old_byte ^ new_byte ) & 0x40 ) ? ( ( new_byte & 0x40 ) ? WHITE1 : BLACK1 ) : NOTHING1;
                data_line_scan[cnt] 	    |= ( ( old_byte ^ new_byte ) & 0x20 ) ? ( ( new_byte & 0x20 ) ? WHITE2 : BLACK2 ) : NOTHING2;
                data_line_scan[cnt--] 	|= ( ( old_byte ^ new_byte ) & 0x10 ) ? ( ( new_byte & 0x10 ) ? WHITE3 : BLACK3 ) : NOTHING3;

                data_line_scan[cnt] 	     = ( ( old_byte ^ new_byte ) & 0x08 ) ? ( ( new_byte & 0x08 ) ? WHITE0 : BLACK0 ) : NOTHING0;
                data_line_scan[cnt] 	    |= ( ( old_byte ^ new_byte ) & 0x04 ) ? ( ( new_byte & 0x04 ) ? WHITE1 : BLACK1 ) : NOTHING1;
                data_line_scan[cnt] 	    |= ( ( old_byte ^ new_byte ) & 0x02 ) ? ( ( new_byte & 0x02 ) ? WHITE2 : BLACK2 ) : NOTHING2;
                data_line_scan[cnt--] 	|= ( ( old_byte ^ new_byte ) & 0x01 ) ? ( ( new_byte & 0x01 ) ? WHITE3 : BLACK3 ) : NOTHING3;

            }
            if( _On_EPD_read_flash != NULL )
            {
                address_offset += Line_Offset( cur_EPD_type_index );
            }
            s_index = y >> 3;
            b_index = ( y % 8 );
            if( ( y % 2 ) == 0 )
            {
                //odd
                data_line_even[line_len - s_index] = scan_lookuptable[b_index];
            }
            else
            {
                //even
                data_line_odd[s_index] = scan_lookuptable[b_index];
            }

            /* Sending data */
            epd_spi_send ( 0x0A, ( uint8_t * )&COG_Line.uint8,
                           COG_parameters[cur_EPD_type_index].data_line_size );

            /* Turn on Output Enable */
            epd_spi_send_byte ( 0x02, 0x07 );

            data_line_odd[s_index] = 0;
            data_line_even[line_len - s_index] = 0;
        }
        /* Count the frame time with offset */
    }
    while ( PartialUpdate_StageTime > get_EPD_timer_value() );

    /* Do while the SysTick timer fulfills the stage time */
    while( PartialUpdate_StageTime > get_EPD_timer_value() );

    /* Stop system timer */
    stop_EPD_timer();
}

/**
 * \brief The Partial Update driving stages for getting Odd/Even data and writing the data
 * from memory array to COG
 *
 * \note
 * - This function for 1.44", 2" and 2.7"
 * - Just run the last stage (of 4 stages of global update) to update image immediately.
 * - If the new data byte is same as previous data byte, send ��Nothing�� data byte which
 *   means the data byte on EPD won��t be changed.
 * �V If the new data byte is different from the previous data byte, send the new data byte.
 * - For more details on the Partial Update driving method, please contact techsupport@pervasivedisplays.com *
 */
static void stage_handle_partial_update( uint8_t const *previous_image_ptr, uint8_t const *new_image_ptr,
        long previous_image_address, long new_image_address )
{
    /** x for horizontal_size loop, y for vertical_size loop, which are EPD pixel size */
    uint16_t x, y, k;
    uint8_t const *r_previous_image_ptr, *r_new_image_ptr;
    uint8_t old_byte, new_byte;
    int16_t scanline_no = 0;
    long address_offset = 0;
    /** Get line data array of EPD size */
    COG_driver_EPDtype_select();


    /* Start a system SysTick timer to ensure the same duration of each stage  */
    start_EPD_timer();
    /* Do while total time of frames exceed stage time
     * Per frame */
    do
    {
        address_offset = 0;
        r_previous_image_ptr = previous_image_ptr;
        r_new_image_ptr = new_image_ptr;
        /* Per data line (vertical size) */
        for ( y = 0; y < COG_parameters[cur_EPD_type_index].vertical_size; y++ )
        {
            if( _On_EPD_read_flash != NULL )
            {
                _On_EPD_read_flash( ( previous_image_address + address_offset ),
                                    previous_lin,
                                    COG_parameters[cur_EPD_type_index].horizontal_size );
                r_previous_image_ptr = previous_lin;

                _On_EPD_read_flash( ( new_image_address + address_offset ),
                                    new_line,
                                    COG_parameters[cur_EPD_type_index].horizontal_size );
                r_new_image_ptr = new_line;

            }
            k = COG_parameters[cur_EPD_type_index].horizontal_size - 1;
            /** Per dot/pixel */
            for ( x = 0; x < COG_parameters[cur_EPD_type_index].horizontal_size; x++ )
            {
                old_byte = *r_previous_image_ptr++;
                new_byte = *r_new_image_ptr++;
                data_line_odd[x]  	 = ( ( old_byte ^ new_byte ) & 0x40 ) ? ( ( new_byte & 0x40 ) ? WHITE3 : BLACK3 ) : NOTHING3;
                data_line_odd[x] 	|= ( ( old_byte ^ new_byte ) & 0x10 ) ? ( ( new_byte & 0x10 ) ? WHITE2 : BLACK2 ) : NOTHING2;
                data_line_odd[x] 	|= ( ( old_byte ^ new_byte ) & 0x04 ) ? ( ( new_byte & 0x04 ) ? WHITE1 : BLACK1 ) : NOTHING1;
                data_line_odd[x] 	|= ( ( old_byte ^ new_byte ) & 0x01 ) ? ( ( new_byte & 0x01 ) ? WHITE0 : BLACK0 ) : NOTHING0;

                data_line_even[k] 	 = ( ( old_byte ^ new_byte ) & 0x80 ) ? ( ( new_byte & 0x80 ) ? WHITE0 : BLACK0 ) : NOTHING0;
                data_line_even[k] 	|= ( ( old_byte ^ new_byte ) & 0x20 ) ? ( ( new_byte & 0x20 ) ? WHITE1 : BLACK1 ) : NOTHING1;
                data_line_even[k] 	|= ( ( old_byte ^ new_byte ) & 0x08 ) ? ( ( new_byte & 0x08 ) ? WHITE2 : BLACK2 ) : NOTHING2;
                data_line_even[k--] 	|= ( ( old_byte ^ new_byte ) & 0x02 ) ? ( ( new_byte & 0x02 ) ? WHITE3 : BLACK3 ) : NOTHING3;

            }
            if( _On_EPD_read_flash != NULL )
            {
                address_offset += Line_Offset( cur_EPD_type_index );
            }
            scanline_no = ( COG_parameters[cur_EPD_type_index].vertical_size - 1 ) - y;
            /* Scan byte shift per data line */
            data_line_scan[( scanline_no >> 2 )] = SCAN_TABLE[( scanline_no % 4 )];


            /* Sending data */
            epd_spi_send ( 0x0A, ( uint8_t * )&COG_Line.uint8,
                           COG_parameters[cur_EPD_type_index].data_line_size );


            /* Turn on Output Enable */
            epd_spi_send_byte ( 0x02, 0x07 );

            data_line_scan[( scanline_no >> 2 )] = 0;
        }
        /* Count the frame time with offset */
        board_reset_watchdog_counter();
    }
    while ( PartialUpdate_StageTime > get_EPD_timer_value() );


    /* Do while the SysTick timer fulfills the stage time */
    while ( PartialUpdate_StageTime > get_EPD_timer_value() );
    /* Stop system timer */
    stop_EPD_timer();
}

/**
 * \brief Complete partial update cycle by giving from the pointer of memory or the address of memory
 */
void EPD_display_partialupdate_Collection( uint8_t const *previous_image_ptr, uint8_t const *new_image_ptr,
        long previous_image_address, long new_image_address,
        EPD_read_memory_handler On_EPD_read_flash )
{
    uint8_t bk_previous_lin[COG_line_Max_Size], bk_new_line[COG_line_Max_Size];
    _On_EPD_read_flash = On_EPD_read_flash;
    previous_lin = &bk_previous_lin[0];
    new_line = &bk_new_line[0];
    epd_spi_send_byte( 0x09, 0xC2 );	//Vcom level

    if( cur_EPD_type_index == EPD_190 || cur_EPD_type_index == EPD_260 )
    {
        stage_handle_partial_update_for_19_26( previous_image_ptr, new_image_ptr, previous_image_address, new_image_address );
        nothing_frame_for_19_26();
        dummy_line();
    }
    else
    {
        stage_handle_partial_update( previous_image_ptr, new_image_ptr, previous_image_address, new_image_address );
        nothing_frame();
        dummy_line();
    }
}

/**
 * \brief Partial update cycle by giving from the pointer of memory
 */
void EPD_display_partialupdate_from_array_prt( uint8_t const *previous_image_ptr, uint8_t const *new_image_ptr )
{
    EPD_display_partialupdate_Collection( previous_image_ptr,
                                          new_image_ptr,
                                          ADDRESS_NULL,
                                          ADDRESS_NULL,
                                          NULL );
}

/**
 * \brief Partial update cycle by giving from the address of memory
 */
void EPD_display_partialupdate_from_flash_prt( long previous_image_address, long new_image_address, EPD_read_memory_handler On_EPD_read_flash )
{
    EPD_display_partialupdate_Collection( NULL,
                                          NULL,
                                          previous_image_address,
                                          new_image_address,
                                          On_EPD_read_flash );
}

/**
 * \brief Write Nothing Frame to COG
 * \note A frame that all D(x,y) are N(01). 0101=0x55=NOTHING
 */
void nothing_frame ( void )
{
    uint16_t i;
    for ( i = 0; i <  COG_parameters[cur_EPD_type_index].horizontal_size; i++ )
    {
        data_line_even[i] = NOTHING;
        data_line_odd[i] = NOTHING;
    }

    for ( i = 0; i < COG_parameters[cur_EPD_type_index].vertical_size; i++ )
    {

        /* Scan byte shift per data line */
        data_line_scan[( i >> 2 )] = SCAN_TABLE[( i % 4 )];

        /* Sending data */
        epd_spi_send ( 0x0A, ( uint8_t * )&COG_Line.uint8, COG_parameters[cur_EPD_type_index].data_line_size );

        /* Turn on Output Enable */
        epd_spi_send_byte ( 0x02, 0x07 );

        data_line_scan[( i >> 2 )] = 0;
    }
}

/**
 * \brief Write Nothing Frame to COG for 1.9" and 2.6"
 */
void nothing_frame_for_19_26 ( void )
{
    uint16_t i;
    uint8_t s_index, b_index, line_len;
    for ( i = 0; i <  ( COG_parameters[cur_EPD_type_index].horizontal_size * 2 ); i++ )
    {
        data_line_scan[i] = NOTHING;
    }
    line_len = ( COG_parameters[cur_EPD_type_index].vertical_size / 8 ) - 1;
    for ( i = 0; i < COG_parameters[cur_EPD_type_index].vertical_size; i++ )
    {

        s_index = i >> 3;
        b_index = ( i % 8 );
        if( ( i % 2 ) == 0 )
        {
            //odd
            data_line_even[line_len - s_index] = scan_lookuptable[b_index];
        }
        else
        {
            //even
            data_line_odd[s_index] = scan_lookuptable[b_index];
        }

        /* Sending data */
        epd_spi_send ( 0x0A, ( uint8_t * )&COG_Line.uint8, COG_parameters[cur_EPD_type_index].data_line_size );

        /* Turn on Output Enable */
        epd_spi_send_byte ( 0x02, 0x07 );

        data_line_odd[s_index] = 0;
        data_line_even[line_len - s_index] = 0;

    }
}

/**
 * \brief BORDER Dummy Line for 1.44",1.9", 2" or 2.6"
 */
void border_dummy_line( void )
{
    uint16_t	i;
    for ( i = 0; i < COG_parameters[cur_EPD_type_index].data_line_size; i++ )
    {
        COG_Line.uint8[i] = 0x00;
    }

    *data_line_border_byte = BORDER_BYTE_B;
    //Write a Border(B) Dummy Line
    epd_spi_send ( 0x0a, ( uint8_t * )&COG_Line.uint8, COG_parameters[cur_EPD_type_index].data_line_size );
    //Turn on OE
    epd_spi_send_byte ( 0x02, 0x07 );

    DelayMs( 40 );

    *data_line_border_byte = BORDER_BYTE_W;
    //Write a Borde(B) Dummy Line
    epd_spi_send ( 0x0a, ( uint8_t * )&COG_Line.uint8, COG_parameters[cur_EPD_type_index].data_line_size );
    //Turn on OE
    epd_spi_send_byte ( 0x02, 0x07 );

}

/**
 * \brief Write Dummy Line to COG
 * \note A line that all Scan Bytes are 0x00
 */
void dummy_line( void )
{
    uint8_t	i;
    for ( i = 0; i < ( COG_parameters[cur_EPD_type_index].vertical_size / 8 ); i++ )
    {
        COG_Line.uint8[i] = 0x00;
    }
    /* Set charge pump voltage level reduce voltage shift */
    epd_spi_send_byte ( 0x04, COG_parameters[cur_EPD_type_index].voltage_level );

    /* Sending data */
    epd_spi_send ( 0x0A, ( uint8_t * )&COG_Line.uint8, COG_parameters[cur_EPD_type_index].data_line_size );

    /* Turn on Output Enable */
    epd_spi_send_byte ( 0x02, 0x07 );
}


/**
 * \brief Power Off COG Driver
 * \note For detailed flow and description, please refer to the COG G2 document Section 6.
 */
uint8_t EPD_power_off ( void )
{

    if( cur_EPD_type_index == EPD_190 || cur_EPD_type_index == EPD_260 )
    {
        nothing_frame_for_19_26();
    }
    else
    {
        nothing_frame();
    }

    if( cur_EPD_type_index == EPD_270 )
    {
        dummy_line();
        DelayMs( 25 );
        EPD_border_low();
        DelayMs ( 200 );
        EPD_border_high();
    }
    else
    {
        border_dummy_line();
        DelayMs ( 200 );
    }

    //Check DC/DC
    if( ( SPI_R( 0x0F, 0x00 ) & 0x40 ) == 0x00 )
    {
        return ERROR_DC;
    }

    epd_spi_send_byte ( 0x0B, 0x00 );

    //Turn on Latch Reset
    epd_spi_send_byte ( 0x03, 0x01 );
    //Power off charge pump Vcom
    epd_spi_send_byte ( 0x05, 0x03 );
    //Power off charge pump neg voltage
    epd_spi_send_byte ( 0x05, 0x01 );
    DelayMs( 120 );
    //Discharge internal SPI
    epd_spi_send_byte ( 0x04, 0x80 );
    //Turn off all charge pump
    epd_spi_send_byte ( 0x05, 0x00 );
    //Turn off OSC
    epd_spi_send_byte ( 0x07, 0x01 );
    DelayMs( 50 );

    EPD_border_low();

    EPD_Vcc_turn_off ();

    DelayMs ( 10 );

    //Pervasive driver bug here change to CS high instead of low
    EPD_cs_high();

    EPD_rst_low();

    EPD_discharge_high ();
    DelayMs ( 150 );
    EPD_discharge_low ();

    return RES_OK;
}

#endif
