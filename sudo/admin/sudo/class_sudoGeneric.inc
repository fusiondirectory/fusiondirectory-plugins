<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003  Cajus Pollmeier
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

class SudoUsersGroupsAttribute extends DialogAttribute
{
  protected $dialogClass = 'UserGroupSelectDialog';

  function addValue ($dn, $entry)
  {
    if (in_array("posixGroup", $entry['objectClass'])) {
      $name = trim("%".$entry['cn'][0]);
    } elseif (isset($entry['uid'][0])) {
      $name = trim($entry['uid'][0]);
    }
    if (!in_array($name, $this->value) && !in_array("!".$name, $this->value)) {
      $this->value[] = $name;
    }
  }

  function getFilterBlackList ()
  {
    $used = array();
    foreach ($this->value as $name) {
      $str = preg_replace("/^!/", "", $name);
      if (preg_match("/^%/", $str)) {
        $used['cn'][] = preg_replace("/^%/", "", $str);
      } else {
        $used['uid'][] = $str;
      }
    }
    return $used;
  }
}

class SudoSystemsAttribute extends SystemsAttribute
{
  function renderButtons ()
  {
    $id = $this->getHtmlId();
    $buttons  = $this->renderInputField(
      'submit', 'add'.$id.'_ALL',
      array('value' => 'ALL')
    );
    return $buttons.parent::renderButtons();
  }

  protected function htmlIds()
  {
    $id = $this->getHtmlId();
    $ids = parent::htmlIds();
    $ids[] = 'add'.$id.'_ALL';
    return $ids;
  }

  function loadPostValue ()
  {
    parent::loadPostValue();
    if ($this->isVisible()) {
      if (isset($_POST['add'.$this->getHtmlId().'_ALL'])) {
        $this->value = array('ALL');
      }
    }
  }

  function addValue ($value, $attrs)
  {
    if ($this->value[0] === 'ALL') {
      unset($this->value[0]);
    }
    parent::addValue($value, $attrs);
  }
}

class defaultRoleAttribute extends BooleanAttribute
{
  function __construct ($defaultValue)
  {
    parent::__construct('Default', 'hidden attribute', 'isDefaultRole', FALSE, $defaultValue, 'noacl');
    $this->setInLdap(FALSE);
    $this->setVisible(FALSE);
  }

  function setValue ($value)
  {
    parent::setValue($value);
    if ($this->getValue()) {
      $this->plugin->attributesAccess['cn']->setValue('defaults');
    }
  }
}

/*! \brief Sudo generic class. Allow setting User/Host/Command/Runas
           for a sudo role object.
 */
class sudo extends simplePlugin
{
  public $objectclasses = array('sudoRole');
  var $mainTab          = TRUE;

  static function plInfo()
  {
    return array(
      'plShortName'   => _('Sudo'),
      'plDescription' => _('Sudo role'),
      'plIcon'        => 'geticon.php?context=applications&icon=sudo&size=48',
      'plSelfModify'  => FALSE,
      'plObjectType'  => array('sudo' => array(
        'name'      => _('Sudo role'),
        'filter'    => 'objectClass=sudoRole',
        'icon'      => 'geticon.php?context=applications&icon=sudo&size=16',
        'ou'        => get_ou('sudoRDN'),
        'tabClass'  => 'sudotabs',
      )),

      'plProvidedAcls' => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array (
      'section1' => array (
        'name'  => _('Role settings'),
        'class' => array('fullwidth'),
        'attrs' => array(
          new defaultRoleAttribute (FALSE),
          new StringAttribute (
            _('Name'), _('Name of the role'),
            'cn', TRUE,
            ''
          ),
          new StringAttribute (
            _('Description'), _('Description for the new sudo role'),
            'description', FALSE
          ),
          new SetAttribute (
            new StringAttribute (
              _('Commands'),
              _('A Unix command with optional command line arguments, potentially including globbing characters (aka wild cards)'),
              'sudoCommand', FALSE
            )
          ),
          new SetAttribute (
            new StringAttribute (
              _('Run as (user)'),
              _('User(s) impersonated by sudo'),
              'sudoRunAsUser', FALSE
            ),
            array('ALL')
          ),
          new SetAttribute (
            new StringAttribute (
              _('Run as (group)'),
              _('Group(s) impersonated by sudo'),
              'sudoRunAsGroup', FALSE
            ),
            array('ALL')
          ),
          new SudoSystemsAttribute (
            _('Systems'), _('A host name, IP address or IP network'),
            'sudoHost', FALSE,
            array('ALL')
          ),
          new SudoUsersGroupsAttribute (
            _('Users and groups'),
            _("A user name, user ID (prefixed with '#'), Unix group (prefixed with '%')"),
            'sudoUser', FALSE
          ),
          new IntAttribute (
            _('Priority'), _('This rule priority compared to others'),
            'sudoOrder', FALSE,
            0, FALSE, 0
          )
        )
      ),
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);
    $this->attributesAccess['isDefaultRole']->setValue($this->is_default());
    $this->attributesAccess['isDefaultRole']->setManagedAttributes(
      array(
        'erase' => array(
          TRUE => array(
            'sudoUser',
            'sudoCommand',
            'sudoHost',
            'sudoRunAsUser',
            'sudoRunAsGroup',
            'sudoOrder',
          )
        ),
        'disable' => array(
          TRUE => array(
            'cn'
          )
        )
      )
    );
  }


  public function set_default($state)
  {
    $this->attributesAccess['isDefaultRole']->setValue($state);
  }

  public function is_default()
  {
    return preg_match("/^defaults$/i", $this->attributesAccess['cn']->getValue());
  }
}

?>
