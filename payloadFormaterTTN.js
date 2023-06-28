function sflt162f(rawSflt16)
    {
    // rawSflt16 is the 2-byte number decoded from wherever;
    // it's in range 0..0xFFFF
    // bit 15 is the sign bit
    // bits 14..11 are the exponent
    // bits 10..0 are the the mantissa. Unlike IEEE format,
    // the msb is explicit; this means that numbers
    // might not be normalized, but makes coding for
    // underflow easier.
    // As with IEEE format, negative zero is possible, so
    // we special-case that in hopes that JavaScript will
    // also cooperate.
    //
    // The result is a number in the open interval (-1.0, 1.0);
    //

    // throw away high bits for repeatability.
    rawSflt16 &= 0xFFFF;

    // special case minus zero:
    if (rawSflt16 == 0x8000)
        return -0.0;

    // extract the sign.
    var sSign = ((rawSflt16 & 0x8000) != 0) ? -1 : 1;

    // extract the exponent
    var exp1 = (rawSflt16 >> 11) & 0xF;

    // extract the "mantissa" (the fractional part)
    var mant1 = (rawSflt16 & 0x7FF) / 2048.0;

    // convert back to a floating point number. We hope
    // that Math.pow(2, k) is handled efficiently by
    // the JS interpreter! If this is time critical code,
    // you can replace by a suitable shift and divide.
    var f_unscaled = sSign * mant1 * Math.pow(2, exp1 - 15);

    return f_unscaled;
    }

function bytes2int(byte1, byte2) {
  return byte1 * 16 * 16 + byte2;
}

function decodeUplink(input) {
  var owTempBytes = bytes2int(input.bytes[1], input.bytes[0]);
  var moistBytes = bytes2int(input.bytes[3], input.bytes[2]);
  var bmeTempBytes = bytes2int(input.bytes[5], input.bytes[4]);
  var bmePressBytes = bytes2int(input.bytes[7], input.bytes[6]);
  var bmeHumidBytes = bytes2int(input.bytes[9], input.bytes[8]);

  var owTemp = sflt162f(owTempBytes) * 200; // factors are same as divisors in code
  var moist = sflt162f(moistBytes) * 4096;
  var bmeTemp = sflt162f(bmeTempBytes) * 200;
  var bmePress = sflt162f(bmePressBytes) * 110000;
  var bmeHumid = sflt162f(bmeHumidBytes) * 100;

  return {
    data: {
      owTemp: owTemp,
      moist: moist,
      bmeTemp: bmeTemp,
      bmePress: bmePress,
      bmeHumid: bmeHumid
    },
    warnings: [],
    errors: []
  };
}
