<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2016 FusionDirectory

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

class userReminderConfig extends simplePlugin
{
  var $objectclasses  = array('fdUserReminderPluginConf');

  static function plInfo()
  {
    return array(
      'plShortName'     => _('User reminder configuration'),
      'plDescription'   => _('FusionDirectory user-reminder script configuration'),
      'plCategory'      => array('configuration'),
      'plObjectType'    => array('smallConfig'),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('user-reminder'),
        'attrs' => array(
          new IntAttribute (
            _('Expiration delay'), _('Days before expiration to send the first mail'),
            'fdUserReminderAlertDelay', TRUE,
            1, FALSE, 15
          ),
          new IntAttribute (
            _('Resend delay'), _('Days before sending a second mail (0 to disable)'),
            'fdUserReminderResendDelay', TRUE,
            0, FALSE, 7
          ),
          new IntAttribute (
            _('Postpone duration'), _('Number of days to postpone the expiration of'),
            'fdUserReminderPostponeDays', TRUE,
            1, FALSE, 15
          ),
          new BooleanAttribute (
            _('Forward alerts'), _('Whether to forward alerts to manager'),
            'fdUserReminderForwardAlert', TRUE,
            TRUE
          ),
          new BooleanAttribute (
            _('Forward confirmations'), _('Whether to forward postpone confirmation to manager'),
            'fdUserReminderForwardConfirmation', TRUE,
            TRUE
          ),
        )
      ),
    );
  }
}
?>
