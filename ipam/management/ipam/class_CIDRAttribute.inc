<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
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
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/

/*! \brief String LDAP attribute that contains an IP (v4 or v6) with netmask in CIDR notation
 *
 */
class CIDRAttribute extends StringAttribute
{
  protected $trim = TRUE;

  function validate ()
  {
    $value = trim($this->getValue());
    list($ip, $bits) = array_pad(explode('/', $value, 2), 2, '');
    $ip = trim($ip);
    $bits = trim($bits);

    if (
      // Incomplete values
      ($ip === '') || ($bits === '') ||
      // Non-numerical suffix
      !preg_match('/^[0-9]+$/', $bits) ||
      // Not an IP at all
      !tests::is_ip($ip) ||
      // Mask size too big for v4
      ((int)$bits > 32 && tests::is_ipv4($ip)) ||
      // Mask size too big for v6
      ((int)$bits > 128)
    ) {
      return msgPool::invalid($this->getLabel(), $this->getValue(), $this->example);
    }

    $bits = (int)$bits;

    // Verify that the IP is the network address (host bits must be zero).
    $packed = @inet_pton($ip);
    if ($packed === FALSE) {
      return msgPool::invalid($this->getLabel(), $this->getValue(), $this->example);
    }

    // Get address length
    $addrLen = strlen($packed); // 4 or 16
    $maxBits = ($addrLen === 4) ? 32 : 128;

    // Build netmask in binary (255.255...) to bin instead of /2)
    $fullBytes = intdiv($bits, 8);
    $remainBits = $bits % 8;
    $mask = str_repeat("\xff", $fullBytes);
  
    if ($remainBits) {
      $mask .= chr((0xff << (8 - $remainBits)) & 0xff);
    }
    if (strlen($mask) < $addrLen) {
      $mask = str_pad($mask, $addrLen, "\0");
    }

    // Compute network part
    $network = '';
    for ($i = 0; $i < $addrLen; $i++) {
      $network .= chr(ord($packed[$i]) & ord($mask[$i]));
    }

    // If original address is not already the network address (host bits set)
    if ($network !== $packed && $bits < $maxBits) {
      $networkStr = inet_ntop($network);
      // Error message
      $example = sprintf(_("Not a network address (host bits set). Expected %s/%d"), $networkStr, $bits);
      return msgPool::invalid($this->getLabel(), $this->getValue(), $example);
    }
  }
}
