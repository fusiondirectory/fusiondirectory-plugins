<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013-2019  FusionDirectory

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

class PgpKeysAttribute extends GenericDialogAttribute
{
  protected $dialogClass      = 'PgpKeySelectDialog';
  protected $height           = 200;
  protected $displayed_values;

  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = [], $acl = '')
  {
    parent::__construct($label, $description, $ldapName, $required, $defaultValue, 'dn', NULL, $acl);
    $this->displayed_values = pgpKeySelect::getPgpFields();
  }

  public function getSelectManagementParameters (): array
  {
    /*
     * objectTypes
     * multiSelect
     * neededAttrs
     * blackList
     * whiteList
     * filterElementDefinitions
     * */
    $neededAttrs = array_merge(
      [
        'objectClass' => '*',
        'dn'          => 'raw',
      ],
      array_map(
        function ($label)
        {
          return '*';
        },
        $this->displayed_values
      )
    );
    return [
      ['pgpKey'],
      TRUE,
      $neededAttrs,
      $this->getFilterBlackList(),
      $this->getFilterWhiteList(),
    ];
  }

  function addValue (string $dn, $entry = NULL)
  {
    if (!in_array($dn, $this->value)) {
      $this->value[]    = $dn;
      $this->displays[] = $entry;
    }
  }

  protected function fillDisplayValue ($i)
  {
    global $config;
    $ldap = $config->get_ldap_link();

    $value = $this->value[$i];
    $ldap->cat($value);
    if (!$ldap->count()) {
      unset($this->value[$i]);
    } else {
      $attrs = $ldap->fetch();
      $this->displays[$i] = $attrs;
    }
  }

  function loadPostValue ()
  {
    if ($this->isVisible()) {
      $id = $this->getHtmlId();
      foreach ($_POST as $name => $value) {
        if (preg_match('/^'.$id.'_del_/', $name)) {
          $key = preg_replace('/^'.$id.'_del_/', '', $name);
          $key = preg_replace('/_[xy]$/', '', $key);
          $this->removeValue($key);
          break;
        }
      }
      if (isset($_POST['add'.$this->getLdapName().'_dialog'])) {
        $this->plugin->openDialog(new $this->dialogClass($this->plugin, $this));
      }
    }
  }

  function renderOnlyFormInput (): string
  {
    $id   = $this->getHtmlId();
    $div  = new divSelectBox($id);
    $div->setHeight($this->height);
    $headers    = array_values($this->displayed_values);
    $headers[]  = '';
    $div->setHeaders($headers);
    // Updates and get display values
    $displays = $this->getDisplayValues();
    foreach ($displays as $key => $display_item) {
      $fields = [];
      foreach (array_keys($this->displayed_values) as $field) {
        if (isset($display_item[$field][0]) && !empty($display_item[$field][0])) {
          $display = $display_item[$field][0];
          if ($field == 'pgpKeySize') {
            // Remove useless 0 in front of the number
            $display = pgpKeySelect::pgpSize($display);
          } elseif ($field == 'pgpKeyCreateTime') {
            $display = pgpKeySelect::pgpDate($display);
          } elseif ($field == 'pgpKeyExpireTime') {
            $display = pgpKeySelect::pgpDate($display);
          } elseif ($field == 'pgpRevoked') {
            $display = pgpKeySelect::pgpRevoked($display);
          } elseif ($field == 'pgpDisabled') {
            $display = pgpKeySelect::pgpRevoked($display);
          }
          $fields[] = ['string' => $display];
        } else {
          $display = '';
          if (($field == 'pgpRevoked') || ($field == 'pgpDisabled')) {
            $display = pgpKeySelect::pgpRevoked($display);
          }
          $fields[] = ['string' => $display];
        }
      }
      $img = '<input type="image" src="geticon.php?context=actions&icon=edit-delete&size=16" name="'.$id.'_del_'.$key.'" class="center"/>&nbsp;';
      $fields[] = ['html' => $img, 'attach' => 'style="border-right:0px;width:20px;"'];
      $div->addEntry($fields);
    }
    $smarty = get_smarty();
    $smarty->assign("div_$id", $div->drawList());
    return '{$div_'.$id.'}'."\n";
  }

  public function htmlIds (): array
  {
    $id = $this->getHtmlId();
    $ids = ['add'.$id.'_dialog'];
    $nb_values = count($this->value);
    for ($i = 0; $i < $nb_values; ++$i) {
      $ids[] = $id.'_del_'.$i;
    }
    return $ids;
  }

  function renderButtons ()
  {
    $id = $this->getHtmlId();
    $buttons  = '<input type="submit" formnovalidate="formnovalidate" name="add'.$id.'_dialog" id="add'.$id.'_dialog"'.
                ($this->disabled ? ' disabled="disabled"' : '').
                ' value="{msgPool type=addButton}"/>';
    return $buttons;
  }
}
