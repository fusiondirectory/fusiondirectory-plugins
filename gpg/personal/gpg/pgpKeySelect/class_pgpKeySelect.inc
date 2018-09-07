<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013-2018  FusionDirectory

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

class pgpKeySelect extends simpleSelectManagement
{
  protected $objectTypes  = array();
  protected $autoFilter   = FALSE;

  protected $departmentBrowser      = FALSE;
  protected $departmentRootVisible  = FALSE;
  protected $baseMode               = FALSE;
  protected $multiSelect            = TRUE;

  function __construct()
  {
    $this->listXMLPath = get_template_path('pgpKeySelect-list.xml', TRUE, dirname(__FILE__));

    parent::__construct();
  }

  function configureHeadpage ()
  {
    parent::configureHeadpage();
    $this->headpage->registerElementFilter('pgpDate', 'pgpKeySelect::filterPgpDate');
    $this->headpage->registerElementFilter('pgpSize', 'pgpKeySelect::filterPgpSize');
    $this->headpage->registerElementFilter('pgpDisabled', 'pgpKeySelect::filterPgpRevoked');
    $this->headpage->registerElementFilter('pgpRevoked', 'pgpKeySelect::filterPgpRevoked');
  }

  /* Build the filter */
  function configureFilter ()
  {
    global $config;

    $ldap = $config->get_ldap_link();
    $ldap->cat('cn=pgpServerInfo,'.$config->current['BASE']);
    if ($pgpServerInfo = $ldap->fetch()) {
      $this->storagePoints = array(
        preg_replace(
          '/'.$config->current['BASE'].'$/', '',
          $pgpServerInfo['pgpBaseKeySpaceDN'][0]
        )
      );
    } else {
      msg_dialog::display(_('Configuration error'), _('You need to configure GPG base dn through the addons section first'));
    }
    $this->filter->setObjectStorage($this->storagePoints);
    $this->filter->category = 'user';
    $attributes = array('pgpKeyID', 'pgpUserID', 'pgpKeyCreateTime', 'pgpKeyExpireTime', 'pgpKeyType', 'pgpKeySize');
    $this->filter->query[0]['backend']      = 'LDAPBlacklist';
    $this->filter->query[0]['filter']       = '$NAME';
    $this->filter->query[0]['attribute']    = $attributes;
    $this->filter->elements['NAME']['set']  = '(|('.join('=*$*)(', $attributes).'=*$*))';
    $this->filter->elements['NAME']['autocomplete']['filter']     = '(|('.join('=*$NAME*)(', $attributes).'=*$NAME*))';
    $this->filter->elements['NAME']['autocomplete']['attribute']  = $attributes;
  }

  static function filterSingleValue($value, $func)
  {
    if (count($value) == 0) {
      return '&nbsp;';
    }
    $return = static::$func($value[0]);
    return ($return === '' ? '&nbsp;' : $return);
  }

  static function filterPgpDate($value = array())
  {
    return static::filterSingleValue($value, 'pgpDate');
  }

  static function filterPgpSize($value = array())
  {
    return static::filterSingleValue($value, 'pgpSize');
  }

  static function filterPgpRevoked($value = array())
  {
    if (empty($value)) {
      return 'No';
    }
    return static::pgpRevoked($value[0]);
  }

  static function pgpDate($date)
  {
    if (empty($date)) {
      return $date;
    }
    return DateTime::createFromFormat('Ymd', substr($date, 0, 8))->format('d.m.Y');
  }

  static function pgpSize($size)
  {
    // Remove useless 0 at the beginning
    return strval(intval($size));
  }

  static function pgpRevoked($value)
  {
    // Remove useless 0 at the beginning
    return ($value == 0 ? 'No' : 'Yes');
  }
}

class PgpKeySelectDialog extends GenericSelectDialog
{
  protected $dialogClass = 'pgpKeySelect';
}

class PgpKeyAttribute extends GenericDialogAttribute
{
  protected $dialogClass      = 'PgpKeySelectDialog';
  protected $height           = 200;
  protected $displayed_values;

  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = array(), $acl = '')
  {
    parent::__construct ($label, $description, $ldapName, $required, $defaultValue, 'dn', NULL, $acl);
    $this->displayed_values = array(
      'pgpKeyID'          => _('Key ID'),
      'pgpUserID'         => _('User ID'),
      'pgpKeyCreateTime'  => _('Creation time'),
      'pgpKeyExpireTime'  => _('Expiration'),
      'pgpKeyType'        => _('Algorithm'),
      'pgpKeySize'        => _('Size'),
      'pgpRevoked'        => _('Revoked'),
      'pgpDisabled'       => _('Disabled')
    );
  }

  function addValue ($dn, $entry)
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

  function renderOnlyFormInput ()
  {
    $id   = $this->getHtmlId();
    $div  = new divSelectBox($id);
    $div->SetHeight($this->height);
    $headers    = array_values($this->displayed_values);
    $headers[]  = '';
    $div->SetHeaders($headers);
    // Updates and get display values
    $displays = $this->getDisplayValues();
    foreach ($displays as $key => $display_item) {
      $fields = array();
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
          $fields[] = array('string' => $display);
        } else {
          $display = '';
          if (($field == 'pgpRevoked') || ($field == 'pgpDisabled')) {
            $display = pgpKeySelect::pgpRevoked($display);
          }
          $fields[] = array('string' => $display);
        }
      }
      $img = '<input type="image" src="geticon.php?context=actions&icon=edit-delete&size=16" name="'.$id.'_del_'.$key.'" class="center"/>&nbsp;';
      $fields[] = array('html' => $img, 'attach' => 'style="border-right:0px;width:20px;"');
      $div->AddEntry($fields);
    }
    $smarty = get_smarty();
    $smarty->assign("div_$id", $div->DrawList());
    return '{$div_'.$id.'}'."\n";
  }

  public function htmlIds()
  {
    $id = $this->getHtmlId();
    $ids = array('add'.$id.'_dialog');
    $nb_values = count($this->value);
    for ($i = 0; $i < $nb_values; ++$i) {
      $ids[] = $id.'_del_'.$i;
    }
    return $ids;
  }

  function renderButtons ()
  {
    $id = $this->getHtmlId();
    $buttons  = '<input type="submit" name="add'.$id.'_dialog" id="add'.$id.'_dialog"'.
                ($this->disabled ? ' disabled="disabled"' : '').
                ' value="{msgPool type=addButton}"/>';
    return $buttons;
  }
}

?>
