<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2015  FusionDirectory

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/

class CertificateFileAttribute extends FileAttribute
{
  static $pemBegin   = "-----BEGIN CERTIFICATE-----";
  static $pemEnd     = "-----END CERTIFICATE-----";

  /* Return the value as an array of values to be displayed in a table columns */
  function getArrayValue ()
  {
    $value = $this->getValue();
    if (!strpos($value, self::$pemBegin)) {
      /* Convert DER to PEM */
      $value = self::$pemBegin."\n".chunk_split(base64_encode($value), 64, "\n").self::$pemEnd."\n";
    }
    $infos = openssl_x509_parse($value, TRUE);
    if (empty($infos)) {
      return array(_('Unknown format'), $this->displayValue($this->getValue()), '');
    }
    $values = array('','','');
    if (isset($infos['subject']['CN'])) {
      $values[0] = $infos['subject']['CN'];
    }
    if (isset($infos['issuer']['CN'])) {
      $values[1] = $infos['issuer']['CN'];
    }
    if (isset($infos['validFrom_time_t'])) {
      $values[2] = date('Y-m-d', $infos['validFrom_time_t']).' -> ';
    } else {
      $values[2] = '? -> ';
    }
    if (isset($infos['validTo_time_t'])) {
      $values[2] .= date('Y-m-d', $infos['validTo_time_t']);
    } else {
      $values[2] .= '?';
    }
    return $values;
  }

  /*! \brief Convert PEM to DER if needed
   */
  function setValue ($value)
  {
    $pattern = '/'.preg_quote(self::$pemBegin, '/').'(.*)'.preg_quote($pemEnd, '/').'/s';
    if (preg_match($pattern, $value, $m)) {
      $value = base64_decode($m[1]);
    }
    parent::setValue($value);
  }
}

class userCertificates extends simplePlugin
{
  var $displayHeader = TRUE;
  var $objectclasses = array('strongAuthenticationUser');

  static function plInfo()
  {
    return array(
      'plShortName'     => _('Certificates'),
      'plDescription'   => _('User certificates'),
      'plSelfModify'    => TRUE,
      'plObjectType'    => array('user'),
      'plIcon'          => '',
      'plSmallIcon'     => '',

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Certificates'),
        'attrs' => array(
          new OrderedArrayAttribute(
            new CertificateFileAttribute (
              '', _('Certificate content'),
              'userCertificate;binary', TRUE
            ),
            FALSE
          ),
        )
      ),
    );
  }

  function __construct (&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);
  }
}
?>
