<?php

require_once('/usr/share/phpqrcode/qrlib.php');
require_once('otphp/OTPInterface.php');
require_once('otphp/TOTPInterface.php');
require_once('otphp/ParameterTrait.php');
require_once('otphp/OTP.php');
require_once('otphp/TOTP.php');
require_once('otphp/Factory.php');
require_once('beberlei/assert/lib/Assert/Assertion.php');
require_once('paragonie/constant_time_encoding/src/Binary.php');
require_once('paragonie/constant_time_encoding/src/EncoderInterface.php');
require_once('paragonie/constant_time_encoding/src/Base32.php');
require_once('beberlei/assert/lib/Assert/AssertionFailedException.php');
require_once('beberlei/assert/lib/Assert/InvalidArgumentException.php');
