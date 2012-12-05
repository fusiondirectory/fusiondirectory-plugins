<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003  Cajus Pollmeier
  Copyright (C) 2011  FusionDirectory

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

class defaultRoleAttribute extends BooleanAttribute
{
  function __construct ($defaultValue)
  {
    parent::__construct('Default', 'hidden attribute', 'isDefaultRole', FALSE, $defaultValue, "noacl");
    $this->setInLdap(FALSE);
    $this->setVisible(FALSE);
  }

  function setValue ($value)
  {
    parent::setValue($value);
    if ($this->getValue()) {
      $this->plugin->attributesInfo['section1']['attrs']['cn']->setValue('defaults');
    }
  }
}

/*! \brief Sudo generic class. Allow setting User/Host/Command/Runas
           for a sudo role object.
 */
class sudo extends simplePlugin
{
  var $displayHeader = FALSE;
  public $objectclasses = array("sudoRole");
  var $cn = "";

  private $nonDefaultAttributes = array('machin', 'bidule');

  /*! \brief  Add ACL object
   *
   * \return Returns the ACL object.
   */
  static function plInfo()
  {
    return array(
      "plShortName"   => _("Sudo"),
      "plDescription" => _("Sudo role"),
      "plIcon"        => _("plugins/sudo/images/sudo.png"),
      "plSelfModify"  => FALSE,
      "plCategory"    => array("sudo" => array("objectClass" => "sudoRole", "description" => _("Sudo role"))),
      "plObjectType"  => array("sudo" => array("name" => _("Sudo role"), "filter" => "objectClass=sudoRole")),

      "plProvidedAcls" => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }


  static function getAttributesInfo ()
  {
    return array (
      'section1' => array (
        'name'  => _('Generic').' - '._('global defaults'),
        'icon' => 'images/rightarrow.png',
        'attrs' => array(
          new defaultRoleAttribute (FALSE),
          new StringAttribute (
            _('Name'),
            _('Name of the role'),
            'cn',
            TRUE,
            ''
          ),
          new StringAttribute (
            _('Description'),
            _('Description for the new sudo role'),
            'description',
            FALSE
          ),
          new SetAttribute (
            new StringAttribute (
              '<img src="images/rocket.png"> '._('Commands'),
              _('A Unix command with optional command line arguments, potentially including globbing characters (aka wild cards)'),
              'sudoCommand',
              FALSE
            )
          ),
          new SetAttribute (
            new StringAttribute (
              '<img src="plugins/users/images/select_user.png"> '._('Run as'),
              _('A user that commands may be run as or a Unix group that contains a list of users that commands may be run as'),
              'sudoRunAs',
              FALSE,
              'ALL'
            )
          ),
          new UsersGroupsAttribute (
            '<img src="plugins/sudo/images/select_workstation.png"> '._('Systems'),
            _('A host name, IP address or IP network'),
            'sudoHost',
            FALSE,
            array('ALL')
          ),
          new UsersGroupsAttribute (
            '<img src="plugins/users/images/select_user.png"> '._('Users and groups'),
            _("A user name, user ID (prefixed with '#'), Unix group (prefixed with '%')"),
            'sudoUser',
            FALSE,
            array('ALL')
          )
        )
      ),
      'section2' => array (
        'name' => _('System trust'),
        'icon' => 'images/lists/locked.png',
        'attrs' => array (
          new SelectAttribute (
            _('Trust mode'),
            _('Trust mode for the role'),
            'trustModel',
            FALSE,
            array('','fullaccess','byhost'),
            '',
            array(_("disabled"), _("full access"),_("allow access to these hosts"))
          ),
          new SetAttribute (
            new StringAttribute (
              " ",
              " ",
              'accessTo',
              FALSE
            )
          )
        )
      )
    );
  }

  function __construct(&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);
    $this->attributesInfo['section1']['attrs']['isDefaultRole']->setValue($this->is_default());
    $this->attributesInfo['section1']['attrs']['isDefaultRole']->setManagedAttributes(
      array(
        'erase' => array(
          TRUE => array(
            'sudoUser',
            'sudoCommand',
            'sudoHost',
            'sudoRunAs',
            'accessTo',
            'trustModel'
          )
        ),
        'disable' => array(
          TRUE => array(
            'cn'
          )
        )
      )
    );
    $this->attributesInfo['section2']['attrs']['trustModel']->setManagedAttributes(
      array (
        'multiplevalues' => array( 'value1' => array('','fullaccess')),
        'erase' => array (
          'value1' => array (
            'accessTo'
          )
        )
      )
    );
  }


  public function set_default($state)
  {
    $this->attributesInfo['section1']['attrs']['isDefaultRole']->setValue($state);
  }

  public function is_default()
  {
    return preg_match("/^defaults$/i",$this->cn); // FIXME : should use attributesInfo and not cn directly
  }

  public function get_cn()
  {
    return $this->cn;
  }

  public static function get_sudoers_ou($config)
  {
    return get_ou("sudoRDN").$config->current['BASE'];
  }
}

?>
