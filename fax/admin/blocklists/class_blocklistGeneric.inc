<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
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

define("BLOCK_LIST_RECEIVE", 0);
define("BLOCK_LIST_SEND", 1);

class blocklistAttribute extends CompositeAttribute
{
  function __construct ($description, $acl = "", $label = "Composite attribute")
  {
    $attributes = array(
      new SelectAttribute(
        _('Type'), _('Type of this list'), 'type', TRUE,
        array(BLOCK_LIST_SEND, BLOCK_LIST_RECEIVE),
        BLOCK_LIST_SEND,
        array(_('send'),_('receive'))
      ),
      new SetAttribute(
        new StringAttribute (
          _('Blocked numbers'),
          _('Numbers can also contain wild cards.'),
          'goFaxBlocklist', TRUE,
          "", "",
          "/^[\/0-9 ()\^\.\$+*-]+$/"
        )
      )
    );
    parent::__construct ($description, "", $attributes, "", "", $acl, $label);
  }

  function getAcl ()
  {
    return 'goFaxBlocklist';
  }

  protected function loadAttrValue ($attrs)
  {
    $list = array();
    $type = BLOCK_LIST_SEND;
    /* Get blocklist type and defined numbers */
    if (isset($attrs['objectClass'])) {
      if (in_array("goFaxSBlock", $attrs['objectClass'])) {
        if (isset($attrs["goFaxSBlocklist"])) {
          for ($i = 0; $i < $attrs["goFaxSBlocklist"]["count"]; $i++) {
            $list[] = $attrs["goFaxSBlocklist"][$i];
          }
        }
      } elseif (in_array("goFaxRBlock", $attrs['objectClass'])) {
        if (isset($attrs["goFaxRBlocklist"])) {
          for ($i = 0; $i < $attrs["goFaxRBlocklist"]["count"]; $i++) {
            $list[] = $attrs["goFaxRBlocklist"][$i];
          }
        }
        $type = BLOCK_LIST_RECEIVE;
      }
    }
    $this->attributes[0]->setValue($type);
    $this->attributes[1]->setValue($list);
  }

  /*! \brief Fill LDAP value in the attrs array
   */
  function fillLdapValue (&$attrs)
  {
    if (!$this->isInLdap()) {
      return;
    }
    /* Type selection */
    $oc = $attrs['objectClass'];
    if ($this->attributes[0]->getValue() == BLOCK_LIST_SEND) {
      $type = "goFaxSBlocklist";
      $this->plugin->objectclasses = array("goFaxSBlock");
      $oc[] = "goFaxSBlock";
      $key = array_search("goFaxRBlock", $oc);
      if ($key !== FALSE) {
        unset($oc[$key]);
      }
    } else {
      $type = "goFaxRBlocklist";
      $this->plugin->objectclasses = array("goFaxRBlock");
      $oc[] = "goFaxRBlock";
      $key = array_search("goFaxSBlock", $oc);
      if ($key !== FALSE) {
        unset($oc[$key]);
      }
    }

    /* Lets cleanup() know what attributes to handle
     */
    $attrs[$type] = $this->attributes[1]->getValue();
    $this->plugin->attributes[] = $type;
    $attrs['objectClass'] = array_values(array_unique($oc));
  }

  function getLdapName()
  {
    if ($this->attributes[0]->getValue() == BLOCK_LIST_SEND) {
      return "goFaxSBlocklist";
    } else {
      return "goFaxRBlocklist";
    }
  }

  function getValue()
  {
    return $this->attributes[1]->getValue();
  }
}

class blocklistGeneric extends simplePlugin
{
  var $displayHeader  = FALSE;
  var $mainTab        = TRUE;

    /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo ()
  {
      return array (
      'section1' => array (
        'name'  => _('Informations'),
        'icon' => 'images/rightarrow.png',
        'attrs' => array (
          new StringAttribute(_('List name'), _('The name of the list'), 'cn', TRUE),
          new StringAttribute(_('Description'), _('A description for the list'), 'description', FALSE),
          new BaseSelectorAttribute(get_ou('faxBlocklistRDN')),
          new blocklistAttribute(_('List of blocked number')),
        )
      )
    );
  }

  /*!
   * \brief Constructor
   *        It will build the select groups choices
   */
  function __construct(&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);
    $this->attributesInfo['section1']['attrs']['cn']->setUnique(TRUE);
  }

  function save ()
  {
    $this->prepare_save();
    $ldap = $this->config->get_ldap_link();
    $old_oc = $this->saved_attributes['objectClass'];
    if (!is_array($old_oc)) {
      $old_oc = array($old_oc);
    }
    if (array_diff($old_oc, $this->attrs['objectClass'])) {
      $ldap->rmdir($this->dn);
    }
    $this->ldap_save();
  }

  function postCopyHook()
  {
    /* We will not be able to rename edited blocklists */
    $this->readonly = TRUE;
  }

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      "plShortName"     => _("Fax blocklists"),
      "plDescription"   => _("The part of the fax plugin managing blacklist"),
      "plSelfModify"    => FALSE,
      "plCategory"      => array( "gofaxlist" => array(
                            "description" => _("Fax blocklists"),
                            "objectClass" => array("goFaxRBlock","goFaxSBlock"))),
      "plObjectType"    => array("faxBlock" => array(
                            "name" => _("Fax blocklists"),
                            "filter" => "(|(goFaxRBlock)(goFaxSBlock))")),
      "plProvidedAcls"  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

}
?>
