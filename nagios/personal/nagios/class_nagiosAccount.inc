<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2005 Guillame Delecourt
  Copyright (C) 2006 Vincent Seynhaeve
  Copyright (C) 2006 Benoit Mortier
  Copyright (C) 2011-2016  FusionDirectory

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

/*! \brief   nagios plugin
  \author  Guillame Delecourt <guillaume@opensides.be>
  \author  Benoit Mortier <benoit.mortier@opensides.be>
  \author  Vincent Seynhaeve <vincent@opensides.be>
  \version 1.00
  \date    25.09.2005

  This class provides the functionality to read and write all attributes
  relevant for nagiosAccount from/to the LDAP. It does syntax checking
  and displays the formulars required.
 */

class nagiosAccount extends simplePlugin
{
  var $objectclasses  = array('lconfContact');

  var $displayHeader  = TRUE;

  static function plInfo()
  {
    return array(
      'plShortName'   => _('Nagios'),
      'plDescription' => _('Nagios account settings'),
      'plIcon'        => 'plugins/nagios/images/icon.png',
      'plSmallIcon'   => 'plugins/nagios/images/iconMini.png',
      'plSelfModify'  => TRUE,
      'plPriority'    => 7,
      'plObjectType'  => array('user'),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo ()
  {
    $prefix = self::get_prefix();
    return array(
      'main' => array(
        'name'  => _('Nagios Account'),
        'attrs' => array(
          new StringAttribute(
            _('Alias'), _('Name of the nagios alias'),
            $prefix.'Alias', TRUE
          ),
          new MailAttribute(
            _('Mail address'), _('Email of the nagios alias'),
            $prefix.'Email', TRUE
          ),
          new SelectAttribute (
            _('Host notification period'), _('Host notification period'),
            $prefix.'ContactHostNotificationPeriod', FALSE,
            array('24x7','24x5','8x5')
          ),
          new SelectAttribute (
            _('Service notification period'), _('Service notification period'),
            $prefix.'ContactServiceNotificationPeriod', FALSE,
            array('24x7','24x5','8x5')
          ),
          new SelectAttribute (
            _('Service notification options'), _('Service notification options'),
            $prefix.'ContactServiceNotificationOptions', FALSE,
            array('w,u,c,r,f','w,u,c,r','w,u,c','c,w','n')
          ),
          new SelectAttribute (
            _('Host notification options'), _('Host notification options'),
            $prefix.'ContactHostNotificationOptions', FALSE,
            array('d,u,r,f','d,u,r','d,u','n')
          ),
          new StringAttribute(
            _('Pager'), _($prefix.' Pager'),
            $prefix.'Pager', FALSE
          ),
          new StringAttribute(
            _('Service notification commands'), _('Service notification commands'),
            $prefix.'ContactServiceNotificationCommands', FALSE
          ),
          new StringAttribute(
            _('Host notification commands'), _('Host notification commands'),
            $prefix.'ContactHostNotificationCommands', FALSE
          )
        )
      )
    );
  }

  static function get_prefix()
  {
    global $config;
    return $config->get_cfg_value('lconfPrefix', 'lconf');
  }

  function __construct (&$config, $dn = NULL, $object = NULL)
  {
    $prefix = self::get_prefix();
    $this->objectclasses = array($prefix.'Contact');
    parent::__construct($config, $dn, $object);
  }
}
