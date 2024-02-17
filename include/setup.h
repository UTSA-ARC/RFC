#pragma once
#include "params/setup_params.h"

#include "globals.h"

#ifdef ARDUINO_TC

    uint8_t AFS_SEL, GFS_SEL;

    int Set_Accel_Range( uint8_t range ) { // Range and sensitivity of accelerometer

        switch ( range ) {

            case 2:                                     // 2Gs of range
                AFS_SEL = 0x00;
                ALSB_Sensitivity = 16384;
            break;

            case 4:                                    // 4Gs of range
                AFS_SEL = 0x08;
                ALSB_Sensitivity = 8192;
            break;

            case 8:                                   // 8Gs of range
                AFS_SEL = 0x10;
                ALSB_Sensitivity = 4096;
            break;

            case 16:                                  // 16Gs of range
                AFS_SEL = 0x18;
                ALSB_Sensitivity = 2048;
            break;

            default:
                Serial.println( "Must input 2, 4, 8, or 16Gs for MPU6050 accelerometer range! Exiting..." );
                return 1;
        }

        return 0;

    }

    int Set_Gyro_Range( uint16_t range ) { // Range and sensitivity of gyroscope

        switch ( range ) {

            case 250:                                    // 250 deg/s of range
                GFS_SEL = 0x00;
                GLSB_Sensitivity = 131;
                break;

            case 500:                                // 500 deg/s of range
                GFS_SEL = 0x08;
                GLSB_Sensitivity = 65.5;
                break;

            case 1000:                               // 1000 deg/s of range
                GFS_SEL = 0x10;
                GLSB_Sensitivity = 32.8;
                break;

            case 2000:                               // 2000 deg/s of range
                GFS_SEL = 0x18;
                GLSB_Sensitivity = 16.4;
                break;

            default:
                Serial.println( "Must input 250, 500, 1000, or 2000 deg/s for MPU6050 gyroscope range! Exiting..." );
                return 1;
        }

        return 0;

    }

    void Init_MPU( const uint8_t mpu_address ) { // Initialize MPU

        Wire.begin();                              // Initialize communication
        Wire.beginTransmission( mpu_address );     // Start communication with MPU6050 // MPU=0x68
        Wire.write( 0x6B );                        // Talk to the register 6B
        Wire.write( 0x00 );                        // Make reset - place a 0 into the 6B register
        Wire.endTransmission( true );              // End the transmission

    }

    void Configure_MPU( const uint8_t accel_config_reg ) { // Configure Accelerometer Sensitivity - Full Scale Range ( default +/- 2g )

        Wire.beginTransmission( MPU_ADDRESS );
        Wire.write( accel_config_reg );                //Talk to the ACCEL_CONFIG register ( 1C hex )
        Wire.write( AFS_SEL );                     //Set the register bits as 00010000 ( +/- 8g full scale range )
        Wire.endTransmission( true );

    }

    void Configure_Gyro( const uint8_t gyro_config_reg ) { // Configure Gyro Sensitivity - Full Scale Range ( default +/- 250deg/s )

        Wire.beginTransmission( MPU_ADDRESS );
        Wire.write( gyro_config_reg );                 // Talk to the GYRO_CONFIG register ( 1B hex )
        Wire.write( GFS_SEL );                     // Set the register bits as 00010000 ( 1000deg/s full scale )
        Wire.endTransmission( true );

    }

    void Configure_BMP( const uint8_t temp_oversampling = BMP3_OVERSAMPLING_8X, const uint8_t press_oversampling = BMP3_OVERSAMPLING_4X, const uint8_t iir_filter_coeff = BMP3_IIR_FILTER_COEFF_3, const uint8_t output_data_rate = BMP3_ODR_50_HZ ) {

        bmp.setTemperatureOversampling( temp_oversampling);
        bmp.setPressureOversampling( press_oversampling );
        bmp.setIIRFilterCoeff( iir_filter_coeff );
        bmp.setOutputDataRate( output_data_rate );

    }

    void Init_Supplimentary_Pins( uint8_t systems_good_pin, uint8_t systems_bad_pin, uint8_t input_voltage_pin, uint8_t vbat_pin ) { // Configure

        pinMode( systems_good_pin, OUTPUT );
        pinMode( systems_bad_pin, OUTPUT );
        pinMode( input_voltage_pin, INPUT );
        pinMode( vbat_pin, INPUT );

    }

    void Init_Paras( const uint8_t* pins, const uint8_t size ) { // Initialize Parachutes

        for (int i = 0; i < size; i++) pinMode( pins[i], OUTPUT );

    }

    String Init_CSV( String file_name = "" ) { // Initialize CSV format
        
        if ( file_name == "" ) file_name = String( month() + '-' + day() + '-' + year() );

        int i = 1;
        while ( SD.exists( ( file_name + ".csv" ).c_str() ) ) { 
        
            file_name += "-" + String( i );
            i++;

        }

        String file_string = file_name + ".csv";

        File myFile = SD.open( file_string.c_str(), FILE_WRITE );
        myFile.println(

            "Time ( RTC ),TimeEnd ( RTC ),Raw Ax ( g ),Raw Ay ( g ),Raw Az ( g ),Ax ( g ),Ay ( g ),Az ( g ),Raw Gx ( deg/s ),Raw Gy ( deg/s ),Raw Gz ( deg/s ),Gx ( deg/s ),Gy ( deg/s ),Gz ( deg/s ),Temperature ( *C ),Pressure ( kpA ),Altitude ( m ),Message"

        );

        myFile.close();

        return file_name;

    }

#endif

// --------------------------------------------------------------------------------------------------------------------

Result Check_Input_Voltage( const uint16_t raw_input_voltage, const float_t max_pin_input_voltage, const float_t min_voltage, const float_t max_voltage ) { // Check if Input voltage is valid

    float_t input_voltage = raw_input_voltage * ( max_pin_input_voltage / 1024.0 ); //* Voltage regulated to 5.0V

    if ( input_voltage < min_voltage ) return { -1, "| !!INPUT VOLTAGE LESS THAN MINIMUM!! |" };
    if ( input_voltage > max_voltage ) return { -2, "| !!INPUT VOLTAGE MORE THAN MAXIMUM!! |" };

    return { 0, "| Safe Input Voltage |" };

}

Result Check_VBAT_Connection( const uint8_t vbat_pin ) { // Check if VBAT is connected

    if ( digitalRead( vbat_pin ) ) return { 0, "| VBAT Connected |"};

    return { -1, "| !!VBAT NOT CONNECTED!! |"};

}

Result Check_Continuity( const uint8_t continuity_src, const uint8_t continuity_gnd ) { // Check continuity between 2 pins

    pinMode( continuity_src, INPUT );
    pinMode( continuity_gnd, OUTPUT );
    
    digitalWrite( continuity_src, HIGH );

    if ( digitalRead( continuity_gnd ) != HIGH ) return { -1, "| Continuity Not Confirmed |" };
    
    return { 0, "| Continuity Not Confirmed |"};

}

Result Check_Pressure_Movement( const float_t pressure, const float_t prev_pressure, const float_t tolerance ) { // Checks the pressure movement

    int H = prev_pressure * ( 1 + tolerance ); // Upperbound
    int L = prev_pressure * ( 1 - tolerance ); // Lowerbound

    if ( pressure < L ) return { 1, "| Pressure Decreasing |" };

    if ( pressure > H ) return { 2, "| Pressure Increasing |" };

    return { 0, "| Constant Pressure |" };

}

Result Check_Surface_Pressure( const float_t pressure, const float_t surface_pressure, const float_t tolerance ) { // Checks if the surface pressure is safe

        int H = surface_pressure * ( 1 + tolerance ); // Upperbound
        int L = surface_pressure * ( 1 - tolerance ); // Lowerbound

        if ( pressure > H || pressure < L ) return { -1, "| !!UNSAFE SURFACE PRESSURE!! |" };

        return { 0, "| Safe Surface Pressure |" };

    return { 0, "| Safe Pressure |" };

}

Result Check_Surface_Tilt( const float_t* surface_gyro, const float_t safe_x_tilt, const float_t safe_y_tilt, const float_t safe_z_tilt, const float_t tolerance ) { // Checks if the surface tilt is safe


    const float_t H[ 3 ] = { // Upperbounds (X,Y,Z)

        ( fabs( safe_x_tilt ) * ( 1 + tolerance ) ),
        ( fabs( safe_y_tilt ) * ( 1 + tolerance ) ),
        ( fabs( safe_z_tilt ) * ( 1 + tolerance ) )

        };

    const float_t L[ 3 ] = { // Lowerbounds (X,Y,Z)

        ( fabs( safe_x_tilt ) * ( 1 - tolerance ) ),
        ( fabs( safe_y_tilt ) * ( 1 - tolerance ) ),
        ( fabs( safe_z_tilt ) * ( 1 - tolerance ) )

        };

    if ( fabs( surface_gyro[ 0 ] ) > H[ 0 ] || fabs( surface_gyro[ 0 ] ) < L[ 0 ] ) return { -10, "| !!DANGEROUS X-AXIS SURFACE TILT!! |" };

    if ( fabs( surface_gyro[ 1 ] ) > H[ 1 ] || fabs( surface_gyro[ 1 ] ) < L[ 1 ] ) return { -20, "| !!DANGEROUS Y-AXIS SURFACE TILT!! |" };

    if ( fabs( surface_gyro[ 2 ] ) > H[ 2 ] || fabs( surface_gyro[ 2 ] ) < L[ 2 ] ) return { -30, "| !!DANGEROUS Z-AXIS SURFACE TILT!! |" };

    return { 0, "| Safe Surface Tilt |" };

}

Result Check_Surface_Accel( const float_t* surface_accel, const float_t safe_x_accel, const float_t safe_y_accel, const float_t safe_z_accel, const float_t tolerance ) { // Checks if the surface accel is safe

    const float_t H[ 3 ] = { // Surface Upperbounds (X,Y,Z)

        fabs( safe_x_accel ) * ( 1 + tolerance ) == 0 ? tolerance : fabs( safe_x_accel ) * ( 1 + tolerance ),
        fabs( safe_y_accel ) * ( 1 + tolerance ) == 0 ? tolerance : fabs( safe_y_accel ) * ( 1 + tolerance ),
        fabs( safe_z_accel ) * ( 1 + tolerance ) == 0 ? tolerance : fabs( safe_z_accel ) * ( 1 + tolerance )

    };

    const float_t L[ 3 ] = {

        fabs( safe_x_accel ) * ( 1 - tolerance ) == 0 ? -1 * tolerance : fabs( safe_x_accel ) * ( 1 - tolerance ),
        fabs( safe_y_accel ) * ( 1 - tolerance ) == 0 ? -1 * tolerance : fabs( safe_y_accel ) * ( 1 - tolerance ),
        fabs( safe_z_accel ) * ( 1 - tolerance ) == 0 ? -1 * tolerance : fabs( safe_z_accel ) * ( 1 - tolerance )

    };

    if ( fabs( surface_accel[ 0 ] ) < L[ 0 ] || fabs( surface_accel[ 0 ] ) > H[ 0 ] ) return { -10, "| !!DANGEROUS X-AXIS SURFACE ACCELERATION!! |" };

    if ( fabs( surface_accel[ 1 ] ) < L[ 1 ] || fabs( surface_accel[ 1 ] ) > H[ 1 ] ) return { -20, "| !!DANGEROUS Y-AXIS SURFACE ACCELERATION!! |" };

    if ( fabs( surface_accel[ 2 ] ) < L[ 2 ] || fabs( surface_accel[ 2 ] ) > H[ 2 ] ) return { -30, "| !!DANGEROUS Z-AXIS SURFACE ACCELERATION!! |" };

    return { 0, "| Safe Accel |" }; // Safe

}

Result Check_Systems( // Checks if systems are safe

    const Data* Values, const Data* Prev_Values,
    const uint8_t* src_pins = {}, const uint8_t* gnd_pins = {}, const uint8_t continuity_arrs_size = 0,
    const uint16_t raw_input_voltage = analogRead( InputVoltagePin ), const float_t max_pin_input_voltage = MAX_PIN_INPUT_VOLTAGE, const float_t min_voltage = MINIMUM_INPUT_VOLTAGE, const float_t max_voltage = MAXIMUM_INPUT_VOLTAGE,
    const uint8_t vbat_pin = PinVBAT,
    const float_t mvmt_press_tol = PMvmntTolerance,
    const float_t surf_press = SafeSurfacePressure, const float_t surf_press_tol = SurfPTolerance,
    const uint8_t systems_good_pin = PinSystemsGood, const uint8_t systems_bad_pin = PinSystemsBad,
    const float_t safe_x_tilt = SafeSurfaceTiltX, const float_t safe_y_tilt = SafeSurfaceTiltY, const float_t safe_z_tilt = SafeSurfaceTiltZ, const float_t tilt_tol = SurfaceTTolerance,
    const float_t safe_x_accel = SafeSurfaceAccelX, const float_t safe_y_accel = SafeSurfaceAccelY, const float_t safe_z_accel = SafeSurfaceAccelZ, const float_t accel_tol = AccTolerance
    
    ) {

    //* Will trigger LED based on error code

    uint8_t results_size = 6 + continuity_arrs_size;

    Result results[ results_size ];

    // Check if connected to sufficient voltage
    
    Result res = Check_Input_Voltage( raw_input_voltage, max_pin_input_voltage, min_voltage, max_voltage );

    results[ 0 ].error = res.error;
    results[ 0 ].message = res.message;

    // Check if VBAT is Connected
    res = Check_VBAT_Connection( vbat_pin );
    results[ 1 ].error = res.error;
    results[ 1 ].message = res.message;

    // Check if pressure is changing
    res = Check_Pressure_Movement( Values->pressure, Prev_Values->pressure, mvmt_press_tol );
    results[ 2 ].error = res.error;
    results[ 2 ].message = res.message;

    // Check current pressure
    res = Check_Surface_Pressure( Values->pressure, surf_press, surf_press_tol );
    results[ 3 ].error = res.error;
    results[ 3 ].message = res.message;

    // Check current tilt
    res = Check_Surface_Tilt( Values->normalized_gyro, safe_x_tilt, safe_y_tilt, safe_z_tilt, tilt_tol );
    results[ 4 ].error = res.error;
    results[ 4 ].message = res.message;
    // results[ 4 ] = { 0, "-" }; //! MPU no werk :'(
    
    // Check current accel
    res = Check_Surface_Accel( Values->normalized_accel, safe_x_accel, safe_y_accel, safe_z_accel, accel_tol );
    results[ 5 ].error = res.error;
    results[ 5 ].message = res.message;

    // results[ 5 ] = { 0, "-" }; //! MPU no werk :'(

    // Check Ejection Charge continuities, if present
    if ( continuity_arrs_size != 0 ) {

        for ( int i = 0; i < continuity_arrs_size; i++ ) results[ 5 + i ] = Check_Continuity( src_pins[ i ], gnd_pins[ i ] );
    
    }

    bool system_good = 1;
    String check_messages = "";

    for ( int i = 0; i < results_size; i++ ) {

        check_messages += results[ i ].message;

        if ( results[ i ].error < 0 ) {
        
            system_good = 0;
    
        }

    }

    if ( !system_good ) {

        digitalWrite( systems_good_pin, 0 );
        digitalWrite( systems_bad_pin, 1 );

        return { -1, ( "| !!Systems Bad!! |" + check_messages ) };

    }

    digitalWrite( systems_good_pin, 1 );
    digitalWrite( systems_bad_pin, 0 );

    return { 0, ( "| Systems Good |" + check_messages ) }; // Safe

}
