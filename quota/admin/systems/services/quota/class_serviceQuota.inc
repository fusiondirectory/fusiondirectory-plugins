<?php
/*
 * This code is part of FusionDirectory (http://www.fusiondirectory.org)
 * Copyright (C) 2010 Antoine Gallavardin
 * Copyright (C) 2011-2013 FusionDirectory
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */

class serviceQuota extends simpleService
{
  var $objectclasses = array('quotaService');

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      'plShortName'   => _('Quota service'),
      'plDescription' => _('Quota service').' ('._('Services').')',
      'plIcon'        => 'plugins/quota/images/iconMini.png',

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'template'  => get_template_path('service_quota_parameters.tpl', TRUE, dirname(__FILE__)),
        'name'      => _('Shares'),
        'attrs'     => array(
          new OrderedArrayAttribute(
            new CharSeparatedCompositeAttribute(
              _('Quotas for the shares this server hosts'),
              'quotaDeviceParameters',
              array(
                new SelectAttribute(
                  _('Device'), _('Device concerned by this quota'),
                  'quotaDevice', TRUE
                ),
                new IntAttribute(
                  _('Block size'), _('Block size of this share (dumpe2fs /dev/sdb3 | grep -i \'Block size\')'),
                  'quotaBlockSize', TRUE,
                  1 /*min*/, FALSE/*no max*/, 4096
                ),
                new StringAttribute(
                  _('Comment'), _('Short comment about this quota'),
                  'quotaDesc', FALSE
                ),
              ),
              ':'
            ),
            FALSE,  // no order
            array(),
            TRUE    // edit button
          )
        )
      ),
      'message' => array(
        'name'      => _('Quota warning message'),
        'attrs'     => array(
          new SelectAttribute(
            _('Charset'), _('Charset use for the message'),
            'quotaMsgCharsetSupport', TRUE
          ),
          new StringAttribute(
            _('Mail command'), _('Command allowing the server to send mails'),
            'quotaMailCommand', FALSE, '/usr/sbin/sendmail -t'
          ),
          new StringAttribute(
            _('Support contact'), _('Email address to contact the administrator'),
            'quotaMsgContactSupport', FALSE
          ),
          new StringAttribute(
            _('From'), _('Email from which the email is sent'),
            'quotaMsgFromSupport', FALSE
          ),
          new StringAttribute(
            _('CC'), _('Send a copy of this email to'),
            'quotaCarbonCopyMail', FALSE
          ),
          new StringAttribute(
            _('Subject'), _('Subject of the sent warning email'),
            'quotaMsgSubjectSupport', FALSE
          ),
          new TextAreaAttribute(
            _('Content'), _('Content of the sent email'),
            'quotaMsgContentSupport', FALSE
          ),
          new StringAttribute(
            _('Signature'), _('Signature to put at the end of the mail'),
            'quotaMsgSignatureSupport', FALSE
          ),
        )
      ),
      'ldap' => array(
        'name'      => _('LDAP message support'),
        'attrs'     => array(
          new SelectAttribute(
            _('LDAP server'), _('The LDAP server to bind to'),
            'quotaLdapServer', TRUE
          ),
          new StringAttribute(
            _('Bind user dn'), _('The user dn to use for bind'),
            'quotaLdapServerUserDn', FALSE
          ),
          new StringAttribute(
            _('Bind user password'), _('The password of the user used for bind'),
            'quotaLdapServerUserPassword', FALSE
          ),
        )
      )
    );
  }

  function __construct (&$config, $dn = NULL, $parent = NULL)
  {
    parent::__construct($config, $dn, $parent);

    $this->attributesAccess['quotaDeviceParameters']->setLinearRendering(FALSE);

    $shares = $this->getShares();
    $this->attributesAccess['quotaDeviceParameters']->attribute->attributes[0]->setChoices(
      array_keys($shares), array_values($shares)
    );

    $ldapServers = $this->getLdapServers();
    $this->attributesAccess['quotaLdapServer']->setChoices(
      array_keys($ldapServers), array_values($ldapServers)
    );

    /* get all encodings types available */
    $charsets = array();
    $configEncodings = $this->config->get_cfg_value('encodings', array());
    foreach ($configEncodings as $str) {
      $arr = explode("=", $str);
      if (count($arr) == 2) {
        $charsets[$arr[0]] = $arr[1];
      } else {
        msg_dialog::display(_('Warning'), sprintf(_('Invalid value in encodings : %s'), $str), WARNING_DIALOG);
      }
    }
    $this->attributesAccess['quotaMsgCharsetSupport']->setChoices(
      array_keys($charsets), array_values($charsets)
    );

  }

  function getShares()
  {
    /* loading of each array of any share delared on this server  */
    $shareInfos = array();
    if (isset($this->attrs['goExportEntry'])) {
      for ($i = 0; $i < $this->attrs['goExportEntry']['count']; $i++) {
        $parts = explode("|", $this->attrs['goExportEntry'][$i]);
        $share_device = $parts[4];
        $share_desc   = sprintf(_('(%s by %s)'), $parts[0], $parts[2]);
        $shareInfos[$share_device] = $share_device.' '.$share_desc;
      }
    }
    return $shareInfos;
  }

  function getLdapServers()
  {
    /* Get List of server with LDAP service */
    $ldapServers = array();
    foreach ($this->config->data['SERVERS']['LDAP'] as $dn => $attrs) {
      $ldapServers[$dn] = $attrs['cn'][0];
    }
    return $ldapServers;
  }
}
?>
