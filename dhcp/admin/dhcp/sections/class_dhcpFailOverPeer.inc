<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2016  FusionDirectory

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

class dhcpFailOverPeer extends dhcpPlugin
{
  public $objectclasses = array('dhcpFailOverPeer');

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('DHCP'),
        'attrs' => array(
          new HostNameAttribute (
            _('Name'), _('Name of this DHCP configuration'),
            'cn', TRUE
          ),
          new StringAttribute (
            _('Primary server'), _('IP address or DNS name of the server playing primary role in DHC Load Balancing and Fail over'),
            'dhcpFailOverPrimaryServer', TRUE
          ),
          new IntAttribute (
            _('Primary port'), _('Port on which primary server listens for connections from its fail over peer (secondary server)'),
            'dhcpFailOverPrimaryPort', TRUE,
            0, FALSE, 519
          ),
          new StringAttribute (
            _('Secondary server'), _('IP address or DNS name of the server playing secondary role in DHC Load Balancing and Fail over'),
            'dhcpFailOverSecondaryServer', TRUE
          ),
          new IntAttribute (
            _('Secondary port'), _('Port on which secondary server listens for connections from its fail over peer (primary server)'),
            'dhcpFailOverSecondaryPort', TRUE,
            0, FALSE, 520
          ),
          new TimeAttribute (
            _('Response delay'), _('Maximum response time in seconds, before Server assumes that connection to fail over peer has failed'),
            'dhcpFailOverResponseDelay', FALSE,
            0, FALSE, 60
          ),
          new IntAttribute (
            _('Unacked updates'), _('Number of BNDUPD messages that server can send before it receives BNDACK from its fail over peer'),
            'dhcpFailOverUnackedUpdates', FALSE,
            0, FALSE, 10
          ),
          new TimeAttribute (
            _('Maximum Client Lead Time'), _('Maximum Client Lead Time configuration in seconds, as defined in DHCP Failover Protocol [FAILOVR]'),
            'dhcpMaxClientLeadTime', FALSE,
            0, FALSE
          ),
          new IntAttribute (
            _('Split'), _('Split between the primary and secondary servers for fail over purpose'),
            'dhcpFailOverSplit', TRUE,
            0, FALSE, 128
          ),
          new StringAttribute (
            _('HashBucketAssignment'), _('HashBucketAssignment bit map for the DHCP Server, as defined in DHC Load Balancing Algorithm [RFC 3074]'),
            'dhcpHashBucketAssignment', TRUE,
            "3600"
          ),
          new TimeAttribute (
            _('Load balance time'), _('Cutoff time in seconds, after which load balance is disabled'),
            'dhcpFailOverLoadBalanceTime', FALSE,
            0, FALSE, 3
          ),
          new TextAreaAttribute (
            _('Comments'), _('Comments about this DHCP object'),
            'dhcpComments', FALSE
          ),
        )
      )
    );
  }
}
