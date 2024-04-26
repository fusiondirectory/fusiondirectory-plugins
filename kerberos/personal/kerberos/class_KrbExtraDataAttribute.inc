<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)

  Copyright (C) 2019-2020 FusionDirectory

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
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
*/

class KrbExtraDataAttribute extends \FusionDirectory\Core\SimplePlugin\Attribute
{

  static function isLastModification($value = '') : bool
  {
    // return true if binary prefix of value
    return ( substr($value, 0, 2) === hex2bin('0002') );
  }

  /* create 'last modification' attribute data */
  static function packLastModification(): string
  {
    $user ='FusionDirectoryWeb';

    global $ui;
    if (isset($ui)) {
      if ($ui->uid != '') {
        $user = $ui->uid.'@FusionDirectory';
      }
    }

    return pack( 'nVa*', 0x0002,  time(), $user ."\0" );
  }

  function renderFormInput (): string
  {
    return '';
  }

  public function htmlIds (): array
  {
    return [];
  }

  protected function arrayValue ($value): array
  {
    $prefix = substr($value, 0, 2);
    if ( $this->isLastModification($value) ) {
      $tag  = _('Last modification');
      //~ - 3-6: are the unix time stamp
      //~ - 7-end: zero-terminated string with the principal of the changer
      $data     = unpack('Vtimestamp/a*user', substr($value, 2));
      $content  = sprintf(_('%s - by "%s"'), date('Y-m-d, H:i:s', $data['timestamp']), $data['user']);
    } elseif ($prefix === hex2bin('0008')) {
      $data     = unpack('vversion', substr($value, 2));
      $tag      = _('Master-key version number');
      $content  = $data['version'];
    } else {
      /* Unknown tag, show hexadecimal data */
      $tag      = bin2hex($prefix);
      $content  = bin2hex(substr($value, 2));
    }
    return [$tag, $content];
  }

  /* Return the value as an array of values to be displayed in a table columns */
  function getArrayValue (): array
  {
    return $this->arrayValue($this->getValue());
  }

  function displayValue ($value): string
  {
    return implode(' - ', $this->arrayValue($value));
  }
}