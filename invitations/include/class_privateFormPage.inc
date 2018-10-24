<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2017-2018 FusionDirectory

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

// Private form code is the same as public form, main difference is this endpoint will be behind SSO
class privateFormPage extends publicFormPage
{
  protected $prefill = TRUE;

  function __construct($interactive = TRUE)
  {
    parent::__construct($interactive);
    /* TODO POC alteredAttributes/providedAttributes/providedMultiValueAttributes */
    $this->providedAttributes = array(
      array('user','givenName'),
      array('user','sn'),
      array('mailAccount','mail'),
      array('supannAccount','eduPersonAffiliation'),
      array('supannAccount','eduPersonPrimaryAffiliation'),
      array('supannAccount','supannListeRouge'),
      array('supannAccount','supannEtuCursusAnnee'),
    );
    $this->providedMultiValueAttributes = array('eduPersonAffiliation');
    /* Set readonly attributes in form (prefill from IDP) */
    $this->alteredAttributes[1] = array(
      'user'          => array('givenName', 'sn'),
      'supannAccount' => array('eduPersonAffiliation', 'eduPersonPrimaryAffiliation', 'supannRefId'),
      'mailAccount'   => array('mail'),
    );
  }

  protected function getPrefilledValues($values = array())
  {
    if ($this->prefill) {
      /* Fill fields from HTTP headers */
      foreach ($this->providedAttributes as list($class, $attr)) {
        if (isset($_SERVER['HTTP_AUTH_'.strtoupper($attr)])) {
          if (!isset($values[$class])) {
            $values[$class] = array();
          }
          if (in_array($attr, $this->providedMultiValueAttributes)) {
            $values[$class][$attr] = explode('; ', $_SERVER['HTTP_AUTH_'.strtoupper($attr)]);
          } else {
            $values[$class][$attr] = $_SERVER['HTTP_AUTH_'.strtoupper($attr)];
          }
        }
      }

      /* Special case: eduPersonPrincipalName */
      if (isset($_SERVER['HTTP_AUTH_'.strtoupper('eduPersonPrincipalName')])) {
        $values['supannAccount']['supannRefId'] = array('{RENATER}'.$_SERVER['HTTP_AUTH_'.strtoupper('eduPersonPrincipalName')]);
        $values['user']['uid']                  = preg_replace('/@[^@]+$/', '', $_SERVER['HTTP_AUTH_'.strtoupper('eduPersonPrincipalName')]);
      }
    }

    return parent::getPrefilledValues($values);
  }
}
