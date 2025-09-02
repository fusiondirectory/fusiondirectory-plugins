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
    // Trim the input value to handle whitespace
    $value = trim($this->getValue());
    // Parse IP and bits from CIDR notation (e.g., 192.168.1.0/24)
    list($ip, $bits) = array_pad(explode('/', $value, 2), 2, '');
    $ip = trim($ip);
    $bits = trim($bits);

    // Basic validation checks
    if (
      // Incomplete values (missing IP or bits)
      ($ip === '') || ($bits === '') ||
      // Non-numerical suffix (bits must be digits)
      !preg_match('/^[0-9]+$/', $bits) ||
      // Not a valid IP address
      !tests::is_ip($ip) ||
      // Mask size too big for IPv4 (max 32)
      ((int)$bits > 32 && tests::is_ipv4($ip)) ||
      // Mask size too big for IPv6 (max 128)
      ((int)$bits > 128)
    ) {
      return msgPool::invalid($this->getLabel(), $this->getValue(), $this->example);
    }

    $bits = (int)$bits;

    // Verify that the IP is the network address (host bits must be zero)
    // Pack the IP into binary format
    $packed = @inet_pton($ip);
    if ($packed === FALSE) {
      return msgPool::invalid($this->getLabel(), $this->getValue(), $this->example);
    }

    // Determine address length (4 for IPv4, 16 for IPv6)
    $addrLen = strlen($packed);
    $maxBits = ($addrLen === 4) ? 32 : 128;

    // Build the netmask in binary format
    $fullBytes = intdiv($bits, 8);
    $remainBits = $bits % 8;
    $mask = str_repeat("\xff", $fullBytes); // Full bytes of 255

    if ($remainBits) {
      // Partial byte: shift 255 left by (8 - remainBits) and mask
      $mask .= chr((0xff << (8 - $remainBits)) & 0xff);
    }
    // Pad mask to address length with null bytes
    if (strlen($mask) < $addrLen) {
      $mask = str_pad($mask, $addrLen, "\0");
    }

    // Compute the network address by ANDing packed IP with mask
    $network = '';
    for ($i = 0; $i < $addrLen; $i++) {
      $network .= chr(ord($packed[$i]) & ord($mask[$i]));
    }

    // If the original IP is not the network address (host bits set), reject
    if ($network !== $packed && $bits < $maxBits) {
      $networkStr = inet_ntop($network);
      // Provide helpful error message with expected network address
      $example = sprintf(_("Not a network address (host bits set). Expected %s/%d"), $networkStr, $bits);
      return msgPool::invalid($this->getLabel(), $this->getValue(), $example);
    }
  }
}
