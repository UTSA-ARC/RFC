
#include "setup.h"

// -------------------------Vector Functions-----------------------

int* Get_Raw_Accel() { // Returns an int vector of the raw acceleration Values from the MPU

    Wire.beginTransmission( MPU );
    Wire.write( 0x3B );                       // Start with register 0x3B ( ACCEL_XOUT_H )
    Wire.endTransmission( false );
    Wire.requestFrom( MPU, 6, true );         // Read 6 registers total, each axis value is stored in 2 registers

    static int result[ 3 ];

    for ( int i = 0; i < 3; i++ ) result[ i ] = ( Wire.read() << 8 | Wire.read() ); // Raw values

    return result;

}

float* Get_Normalized_Accel( int* raw_accel ) { // Returns the normalized acceleration Values from the MPU

    static float normalized_accel[ 3 ];

    for ( int i = 0; i < 3; i++ ) normalized_accel[ i ] = raw_accel[ i ] / ( ALSB_Sensitivity / 1.0f );

    return normalized_accel;

}

int* Get_Raw_Gyro() { // Returns an int vector containing the raw gyrospocic Values from the MPU

    Wire.beginTransmission( MPU );
    Wire.write( 0x43 );                     // Gyro data first register address 0x43
    Wire.endTransmission( false );
    Wire.requestFrom( MPU, 6, true );       // Read 4 registers total, each axis value is stored in 2 registers

    static int raw_gyro[ 3 ];

    for ( int i = 0; i < 3; i++ ) raw_gyro[ i ] = ( Wire.read() << 8 | Wire.read() ) ;

    return raw_gyro;

}

float* Get_Normalized_Gyro( int* raw_gyro ) { // Returns an int vector containing the normalized gyrospocic Values from the MPU

    static float normalized_gyro[ 3 ];

    for ( int i = 0; i < 3; i++ ) normalized_gyro[ i ] = raw_gyro[ 0 ] / ( GLSB_Sensitivity / 1.0f );

    return normalized_gyro;

}

// -----------------------Struct Functions------------------------------------------

Data Get_All_Values_INT() {

    Data data;

    int i = 0;
    // Get Values from Accelorometer
    int *raw_a_ptr = Get_Raw_Accel();
    int *raw_g_ptr = Get_Raw_Gyro();
    float *norm_a_ptr = Get_Normalized_Accel( raw_a_ptr );
    float *norm_g_ptr = Get_Normalized_Gyro( raw_g_ptr );

    for ( i = 0; i < 3; i++ ) {

        data.raw_accel[ i ] = raw_a_ptr[ i ];
        data.raw_gyro[ i ] = raw_g_ptr[ i ];
        data.normalized_accel[ i ] = norm_a_ptr[ i ];
        data.normalized_gyro[ i ] = norm_g_ptr[ i ];

    }

    time_t time_now = now();
    data.time = (

        String( hour( time_now ) ) + ":" +
        String( minute( time_now ) < 10 ? ( '0' + String( minute( time_now ) ) ) : minute( time_now ) ) + ":" +
        String( second( time_now ) < 10 ? ( '0' + String( second( time_now ) ) ) : second( time_now ) )

        );

    data.temperature = ( bmp.temperature  );
    data.pressure = ( bmp.pressure  );
    data.altitude = ( bmp.readAltitude( SEALEVELPRESSURE_HPA )  );

    return data;

}

// -----------------------Data Handling Functions-----------------------------------

void Print_All_Values( Data& Values ) { // Print the Values on the serial monitor

    Serial.begin( 115200 ); // Open Serial Port

    String output = ""; // init output string

    output.append( "Time ( S ): " + Values.time + '\n' );

    output.append(

        "Raw Acceleration ( X, Y, Z ): " +
        String( Values.raw_accel[ 0 ] / 1.0f, 2 ) + ',' +
        String( Values.raw_accel[ 1 ] / 1.0f, 2 ) + ',' +
        String( Values.raw_accel[ 2 ] / 1.0f, 2 ) + '\n'

        );

    output.append(

        "Normalized Acceleration ( X, Y, Z ): " +
        String( Values.normalized_accel[ 0 ] / 1.0f, 2 ) + ',' +
        String( Values.normalized_accel[ 1 ] / 1.0f, 2 ) + ',' +
        String( Values.normalized_accel[ 2 ] / 1.0f, 2 ) + '\n'

        );

    output.append(

        "Raw GyroRange ( X, Y, Z ): " +
        String( Values.raw_gyro[ 0 ] / 1.0f, 2 ) + ',' +
        String( Values.raw_gyro[ 1 ] / 1.0f, 2 ) + ',' +
        String( Values.raw_gyro[ 2 ] / 1.0f, 2 ) + '\n'

        );

    output.append(

        "Normalized Gyro Range ( X, Y, Z ): " +
        String( Values.normalized_gyro[ 0 ] / 1.0f, 2 ) + ',' +
        String( Values.normalized_gyro[ 1 ] / 1.0f, 2 ) + ',' +
        String( Values.normalized_gyro[ 2 ] / 1.0f, 2 ) + '\n'

        );

    output.append( "\n\nNow reading BMP390...\n" );
    output.append( "Tempurature ( C ): " + String( Values.temperature / 1.0f ) + '\n' );
    output.append( "Pressure ( kPa ): " + String( Values.pressure / 1.0f ) + '\n' );
    output.append( "Altitude ( m ): " + String( Values.altitude / 1.0f ) + '\n' );

    Serial.println( output ); // Print output to screen

    Serial.end(); // End Serial Transmission

}

void Write_All_Values_To_SD( Data& Values ) { // Records values to Sd card

    String file_string = String( month() + '-' + day() + '-' + year() ) + ".csv";
    File myFile = SD.open( file_string.c_str(), FILE_WRITE );

    //* Time ( seconds ),
    //* Raw Ax ( g ),Raw Ay ( g ),Raw Az ( g ),
    //* Ax ( g ),Ay ( g ),Az ( g ),
    //* Raw Gx ( deg/s ),Raw Gy ( deg/s ),Raw Gz ( deg/s ),
    //* Gx ( deg/s ),Gy ( deg/s ),Gz ( deg/s ),
    //* Temperature ( *C ),Pressure ( kpA ),Altitude ( m ),
    //* Message

    String output = "";

    output += ( Values.time + ',' );

    int i;

    for ( i = 0; i < 3; i++ ) output += ( String( Values.raw_accel[ i ] / 1.0f ) + ',' );

    for ( i = 0; i < 3; i++ ) output += ( String( Values.normalized_accel[ i ] / 1.0f ) + ',' );

    for ( i = 0; i < 3; i++ ) output += ( String( Values.raw_gyro[ i ] / 1.0f ) + ',' );

    for ( i = 0; i < 3; i++ ) output += ( String( Values.normalized_gyro[ i ] / 1.0f ) + ',' );

    output += ( String( Values.temperature / 1.0f ) + ',' );

    output += ( String( Values.pressure / 1.0f ) + ',' );

    output += ( String( Values.altitude / 1.0f ) + ',' );

    output += ( Values.message );

    myFile.println( output );

    myFile.close();

}

void Record_Data( Data& Values ) { // Prints data to screen and saves it to file

    Print_All_Values( Values );
    Write_All_Values_To_SD( Values );

}

// -----------------------Parachute Functions---------------------------------------

void Arm_Parachute( int schute ) { // Arms Parachute

   Paras_Armed[ schute ] = 1;

}

Result Launch_Parachute( int schute ) { // Launches Parachute

    switch ( schute ) {

        case 0: // Main Schute

            if ( Paras_Armed[ schute ] ) {

                digitalWrite( PinMain, HIGH );
                return { 0, "!!MAIN SCHUTE LAUNCHED!!" };

                }

            else return { 1, "MAIN Schute Not Deployed!" };

        case 1: // Drouge Schute

            if ( Paras_Armed[ schute ] ) {

                digitalWrite( PinDrouge, HIGH );
                return { 0, "!!DROUGE SCHUTE LAUNCHED" };

            }

            else return { 2, "DROUGE Schute Not Deployed!" };

        default:
            return { -1, "Not a Valid Schute!" };

    }

}

// -----------------------Internal Trigger Functions--------------------------------

Result Check_Altitude( int altitude, int prev_altitude, int apogee ) { // Checks if altitude is safe/at apogee

    if ( altitude > SafeAltitude  ) {

        if ( altitude <= prev_altitude ) {

            if ( altitude >= ( prev_altitude * ( 1 - ATolerance ) ) ) return { 1, "!!AT APOGEE!!" }; // Around Apogee

            if ( ( altitude <= apogee - MainParaADelta ) &&
                 ( altitude >= apogee - ( MainParaADelta * ( 1 + ATolerance ) ) ) ) return { 2, "!!MAIN PARACHUTE ALTITUDE REACHED!!" };

        }

        else return { 0, "Safe Altitude" }; // Safe

    }

    return { -1, "unsafe altitude" }; // Unsafe

}

Result Check_Pressure_Delta( float pressure, float prev_pressure ) { // Checks pressure delta

    int H = prev_pressure * ( 1 + PTolerance ); // Upperbound
    int L = prev_pressure * ( 1 - PTolerance ); // Lowerbound

    if ( pressure < L ) return { 1, "Pressure Decreasing" };

    if ( pressure > H ) return { 2, "Pressure Increasing" };

    return { 0, "Constant Pressure" };

}

Result Check_Pressure( float pressure, bool surface = 0 ) {
 
    if ( surface ) {

        int H = SurfacePressure * ( 1 + SurfPTolerance ); // Upperbound
        int L = SurfacePressure * ( 1 - SurfPTolerance ); // Lowerbound

        if ( pressure > H || pressure < L ) return { 0, "!!UNSAFE SURFACE PRESSURE!!" };

        return { 0, "Safe Surface Pressure" };

    }

    return { 0, "Safe Pressure" };

}

Result Check_Tilt( float* gyro, float* prev_gyro, bool surface = false ) { // Checks if tilt is safe

    float H[ 3 ] = { // Upperbounds (X,Y,Z)

        ( prev_gyro[ 0 ] * ( 1 + TTolerance ) ),
        ( prev_gyro[ 1 ] * ( 1 + TTolerance ) ),
        ( prev_gyro[ 2 ] * ( 1 + TTolerance ) )

        };

    float L[ 3 ] = { // Lowerbounds (X,Y,Z)

        ( prev_gyro[ 0 ] * ( 1 - TTolerance ) ),
        ( prev_gyro[ 1 ] * ( 1 - TTolerance ) ),
        ( prev_gyro[ 2 ] * ( 1 - TTolerance ) )

        };

    if ( surface ) {

        if ( abs( gyro[ 0 ] ) > SurfaceTiltX ) return { -10, "!!DANGEROUS X-AXIS SURFACE TILT!!" };

        if ( abs( gyro[ 1 ] ) > SurfaceTiltY ) return { -20, "!!DANGEROUS Y-AXIS SURFACE TILT!!" };

        if ( abs( gyro[ 2 ] ) > SurfaceTiltZ ) return { -30, "!!DANGEROUS Z-AXIS SURFACE TILT!!" };

        return { 0, "Safe Surface Tilt" };

    }

    if ( abs( gyro[ 0 ] ) > SafeTiltX ) return { -1, "!!DANGEROUS X-AXIS TILT!!" };

    if ( abs( gyro[ 1 ] ) > SafeTiltY ) return { -2, "!!DANGEROUS Y-AXIS TILT!!" };

    if ( abs( gyro[ 2 ] ) > SafeTiltZ ) return { -3, "!!DANGEROUS Z-AXIS TILT!!" };

    if ( gyro[ 0 ] < L[ 0 ] || gyro[ 0 ] > H[ 0 ] ) return { 1, "X-AXIS TILT DETECTED" };

    if ( gyro[ 1 ] < L[ 1 ] || gyro[ 1 ] > H[ 1 ] ) return { 2, "Y-AXIS TILT DETECTED" };

    if ( gyro[ 2 ] < L[ 2 ] || gyro[ 2 ] > H[ 2 ] ) return { 3, "Z-AXIS TILT DETECTED" };

    return { 0, "safe tilt"};

}

Result Check_Accel( float* accel, float* prev_accel, bool surface = false ) { // Checks if accel is correct

    if ( surface ) {

        float Hs[ 3 ] = { // Surface Upperbounds (X,Y,Z)
            
            SurfaceAccelX * ( 1 + ATolerance ) == 0 ? ATolerance : SurfaceAccelX * ( 1 + ATolerance ),
            SurfaceAccelY * ( 1 + ATolerance ) == 0 ? ATolerance : SurfaceAccelY * ( 1 + ATolerance ),
            SurfaceAccelZ * ( 1 + ATolerance ) == 0 ? ATolerance : SurfaceAccelZ * ( 1 + ATolerance )

        };

        float Ls[ 3 ] = {

            SurfaceAccelX * ( 1 - ATolerance ) == 0 ? -1 * ATolerance : SurfaceAccelX * ( 1 - ATolerance ),
            SurfaceAccelY * ( 1 - ATolerance ) == 0 ? -1 * ATolerance : SurfaceAccelY * ( 1 - ATolerance ),
            SurfaceAccelZ * ( 1 - ATolerance ) == 0 ? -1 * ATolerance : SurfaceAccelZ * ( 1 - ATolerance )

        };

        if ( accel[ 0 ] < Ls[ 0 ] || accel[ 0 ] > Hs[ 0 ] ) return { -10, "!!DANGEROUS X-AXIS SURFACE ACCELERATION!!" };

        if ( accel[ 1 ] < Ls[ 1 ] || accel[ 1 ] > Hs[ 1 ] ) return { -20, "!!DANGEROUS Y-AXIS SURFACE ACCELERATION" };

        if ( accel[ 2 ] < Ls[ 2 ] || accel[ 2 ] > Hs[ 2 ] ) return { -30, "!!DANGEROUS Z-AXIS SURFACE ACCELERATION" };

    }

    else {

        float H[ 3 ] = { // Upperbounds (X,Y,Z)

            prev_accel[ 0 ] * ( 1 + ATolerance ) ,
            prev_accel[ 1 ] * ( 1 + ATolerance ) ,
            prev_accel[ 2 ] * ( 1 + ATolerance )

        };

        float L[ 3 ] = { // Lowerbounds (X,Y,Z)

            prev_accel[ 0 ] * ( 1 - ATolerance ),
            prev_accel[ 1 ] * ( 1 - ATolerance ),
            prev_accel[ 2 ] * ( 1 - ATolerance )

        };

        if ( abs( accel[ 0 ] ) > SafeAccelX ) return { -1, "!!DANGEROUS X-AXIS ACCELERATION!!" };

        if ( abs( accel[ 1 ] ) > SafeAccelY ) return { -2, "!!DANGEROUS Y-AXIS ACCEL!!" };

        if ( abs( accel[ 2 ] ) > SafeAccelX ) return { -3, "!!DANGEROUS Z-AXIS ACCEL!!" };

        if ( accel[ 0 ] < L[ 0 ] || accel[ 0 ] > H[ 0 ] ) return { 1, "EXTRA X-AXIS ACCEL DETECTED" };

        if ( accel[ 1 ] < L[ 1 ] || accel[ 1 ] > H[ 1 ] ) return { 2, "EXTRA Y-AXIS ACCEL DETECTED" };

        if ( accel[ 2 ] < L[ 2 ] || accel[ 2 ] > H[ 2 ] ) return { 2, "EXTRA Z-AXIS ACCEL DETECTED" };

    }

    return { 0, "Safe Accel" }; // Safe

}

// -----------------------External Trigger Functions--------------------------------

Result Check_Systems( Data Values, Data Prev_Values ) { // Checks if systems are safe

    //* Will trigger LED based on error code
    
    Result results[ 6 ];

    pinMode( PinSystemsGood, OUTPUT );
    pinMode( PinSystemsBad, OUTPUT );
    pinMode( PinInputVoltage, INPUT );
    pinMode( PinVBAT, INPUT );

    // Check if connected to sufficient voltage
    results[ 0 ] = Check_Input_Voltage( analogRead( PinInputVoltage ) );
    
    // Check if VBAT is Connected
    results[ 1 ] = Check_VBAT_Connection();

    results[ 2 ] = Check_Pressure_Delta( Values.pressure, Prev_Values.pressure );

    results[ 3 ] = Check_Pressure( Values.pressure, true );

    results[ 4 ] = Check_Tilt( Values.normalized_gyro, Prev_Values.normalized_gyro, true );

    results[ 5 ] = Check_Accel( Values.normalized_accel, Prev_Values.normalized_accel, true );

    for ( int i = 0; i < 4; i++ ) {

        if ( results[ i ].error < 0 ) {

            digitalWrite( PinSystemsGood, 0 );
            digitalWrite( PinSystemsBad, 1 );
            
            return { -1, ( "!!SYSTEMS BAD!! - " + results[ i ].message ) };

        }

    }

    digitalWrite( PinSystemsBad, 0 );
    digitalWrite( PinSystemsGood, 1 );

    return { 0, "Systems Good" }; // Safe

}
