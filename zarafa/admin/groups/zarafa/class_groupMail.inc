<?php

/*
 * This code is part of GOsa (http://www.gosa-project.org)
 * Copyright (C) 2009-2010 Bas Tichelaar
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


class MailSelectDialog
{
  private $mailSelect;
  private $mailsAttribute;

  function __construct ($simplePlugin,$mailsAttribute)
  {
    $this->mailsAttribute = $mailsAttribute;
    $this->mailSelect = new mailAddressSelect($simplePlugin->config, get_userinfo());
  }

  function execute ()
  {
    if (isset($_POST['mailAddressSelect_cancel'])) {
      return FALSE;
    }
    $used  = array();
    $used['mail'] = $this->mailsAttribute->getForbiddenValues();
    print_r($used);
    if (isset($_POST['mailAddressSelect_save'])) {
      $list = $this->mailSelect->save();
      foreach ($list as $entry) {
        $val = $entry['mail'][0];
        $this->mailsAttribute->addValue($val);
      }
      return FALSE;
    }

    // Build up blocklist
    session::set('filterBlacklist', $used);
    return $this->mailSelect->execute();
  }
}


class MailAttribute extends SetAttribute
{

  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValues = array(), $acl = "")
  {
    parent::__construct(new StringAttribute($label, $description, $ldapName, $required, "", $acl, "/^.*@.*$/"), $defaultValues);
  }

  function getDisplayValues ()
  {
    return $this->value;
  }

  function addValue($value)
  {
    $this->postValue = $this->value;
    $this->addPostValue($value);
    $this->applyPostValue();
  }

  function addPostValue($value)
  {
    if (!in_array($value, $this->getForbiddenValues())) {
      $this->postValue[] = $value;
    }
  }

  function loadPostValue ()
  {
    if (isset($_POST["add".$this->getLdapName()."_dialog"])) {
      $this->postValue = $this->value;
      $this->plugin->openDialog(new MailSelectDialog($this->plugin,$this));
    } else {
      parent::loadPostValue();
    }
  }

  function getForbiddenValues ()
  {
    $forbidden = array_values($this->getValue());
    $forbidden = array_merge($forbidden, array_values($this->plugin->attributesInfo['section1']['attrs']['gosaMailAlternateAddress']->getValue()));
    $forbidden[] = $this->plugin->attributesInfo['section1']['attrs']['mail']->getValue();
    return $forbidden;
  }

  function renderButtons ()
  {
    $id = $this->getLdapName();
    $buttons  = '<input type="submit" name="add'.$id.'"'.
                ($this->disabled? ' disabled="disabled"':'').
                ' value="{msgPool type=addButton}"/>';
    $buttons .= '<input type="submit" name="add'.$id.'_dialog"'.
                ($this->disabled? ' disabled="disabled"':'').
                'value="{t}Add local{/t}"'.
                '/>';
    $buttons .= '<input type="submit" name="del'.$id.'"'.
                ($this->disabled? ' disabled="disabled"':'').
                ' value="{msgPool type=delButton}"/>';
    return $buttons;
  }
}



class zarafamailgroup extends simplePlugin
{
  var $displayHeader  = TRUE;
  var $objectclasses  = array("gosaMailAccount");
  /* Return plugin informations for acl handling  */
  static function plInfo()
  {
    return (array(
          "plShortName"    => _("Mail"),
          "plDescription"  => _("Group mail"),
          "plSelfModify"   => FALSE,
          "plDepends"      => array(),
          "plPriority"     => 10,
          "plSection"      => array("administration"),
          "plCategory"     => array("groups"),
          "plProvidedAcls" => parent::generatePlProvidedAcls(self::getAttributesInfo())
      ));
  }


  /*!
   * \brief Constructor of the freeradiusAccount
   *        It will build the select groups choices
   */
  function __construct(&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->search("(objectClass=goMailServer)",array('cn'));
    $servercn = array();
    while ($attrs = $ldap->fetch()) {
        $servercn[] = $attrs['cn'][0];
    }
    $this->attributesInfo['section1']['attrs']['gosaMailServer']->setChoices($servercn);
  }


  static function getAttributesInfo ()
  {
    return array(
      // Attributes are grouped by section
      'section1' => array(
        'name'  => _('Generic'),
        'attrs' => array(
          new StringAttribute (
            _('Primary address'),
            _('The primary mail address'),
            'mail',
            TRUE,
            '',
            '',
            "/^[^@]*@[^@]*$/"
          ),
          new SelectAttribute (
            _('Server'),
            _('Server mail for zarafa'),
            'gosaMailServer',
            TRUE,
            array("")
          ),
          new SetAttribute (
            new StringAttribute (
              _('Alternative addresses'),
              _('Alternative mail addresses for zarafa group'),
              'gosaMailAlternateAddress',
              FALSE,
              ''
            )
          ),
            new MailAttribute (
              _('Forward messages to non group members'),
              _(''),
              'gosaMailForwardingAddress'
          )
        )
      )
    );
  }
}
