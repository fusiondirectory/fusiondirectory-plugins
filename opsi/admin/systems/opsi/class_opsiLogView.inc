<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2016-2017  FusionDirectory

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

class opsiLogView extends simplePlugin
{
  var $mac;
  var $o_queue;

  var $displayHeader  = FALSE;

  static function getAttributesInfo ()
  {
    return array(
      'logs' => array(
        'name'  => _('Available logs'),
        'class' => array('fullwidth'),
        'attrs' => array(
          new SelectAttribute(
            _('Available logs'), _('Available log files'),
            'available_logs', FALSE
          ),
          new DisplayAttribute(
            'test', _('Content of the selected log'),
            'display_log'
          ),
        )
      )
    );
  }

  /* Return plugin information */
  static function plInfo()
  {
    return array(
      'plShortName'   => _('OPSI Logs'),
      'plDescription' => _('OPSI Logs Viewer'),
      'plPriority'    => 30,
      'plObjectType'  => array('workstation'),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    /* Try to fetch logs for the given event (mac) */
    $this->o_queue = new supportDaemon();

    if ($dn == '') {
      $dn = 'new';
    }
    parent::__construct($dn, $object, $parent, $mainTab);

    /* Get correct macAddress. Check if an event is given or a ldap object. */
    if (isset($this->attrs['macAddress'][0])) {
      $this->mac = $this->attrs['macAddress'][0];
    }

    if ($this->mac != '') {
      /* Query for log files */
      $res = $this->o_queue->append_call('System.list_logs', $this->mac, array('args' => array()));
      if ($this->o_queue->is_error()) {
        msg_dialog::display(_("Error"), $this->o_queue->get_error(), ERROR_DIALOG);
      }
      var_dump($res);

      /* Check if there is at least one log file */
      if (is_array($res)) {
        $this->attributesAccess['available_logs']->setChoices($res);
      }
    }

    $this->attributesAccess['available_logs']->setSubmitForm('loadLog');
    $this->attributesAccess['available_logs']->setInLdap(FALSE);
    $this->attributesAccess['display_log']->setAllowHTML(TRUE);

    $this->is_account     = $this->is_this_account($this->attrs);
    $this->ignore_account = FALSE;
  }

  function is_this_account($attrs)
  {
    /* Will not work when called from parent constructor (or when $attrs is not us) */
    return (count($this->attributesAccess['available_logs']->getChoices()) > 0);
  }

  function loadLog()
  {
    $res = $this->o_queue->append_call('System.get_log', $this->mac, array('args' => array($this->available_logs)));
    if ($this->o_queue->is_error()) {
      msg_dialog::display(_("Error"), $this->o_queue->get_error(), ERROR_DIALOG);
    } else {
      $value = nl2br(htmlentities($res, ENT_COMPAT, 'UTF-8'));
      $this->attributesAccess['display_log']->setPostValue($value);
      $this->attributesAccess['display_log']->setValue($value);
    }
  }

  function save()
  {
  }
}
?>
