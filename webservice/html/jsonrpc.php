<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013  FusionDirectory

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

/*!
 * \file jsonrpc.php
 * \brief This file is a webservice for FusionDirectory
 *
 * It's a JSON-RPC service usually used through HTTPS. It should be put in the html folder of FusionDirectory
 * Then the url of the webservice will be the url of your FusionDirectory instance followed by /jsonrpc.php
 */

function authenticateHeader($message = 'Authentication required')
{
  header('WWW-Authenticate: Basic realm="FusionDirectory"');
  header('HTTP/1.0 401 Unauthorized');
  echo "$message\n";
  exit;
}

require_once("../include/php_setup.inc");
require_once("functions.inc");
require_once("variables.inc");
require_once("jsonrpcphp/jsonRPCServer.php");

function initiateRPCSession($id = NULL, $ldap = NULL, $user = NULL, $pwd = NULL)
{
  global $config, $class_mapping, $BASE_DIR, $ui, $ssl;

  session::start($id);

  /* Reset errors */
  reset_errors();
  /* Check if CONFIG_FILE is accessible */
  if (!is_readable(CONFIG_DIR."/".CONFIG_FILE)) {
    die(sprintf(_("FusionDirectory configuration %s/%s is not readable. Aborted."), CONFIG_DIR, CONFIG_FILE));
  }

  /* Initially load all classes */
  $class_list = get_declared_classes();
  foreach ($class_mapping as $class => $path) {
    if (!in_array($class, $class_list)) {
      if (is_readable("$BASE_DIR/$path")) {
        require_once("$BASE_DIR/$path");
      } else {
        die(sprintf(_("Cannot locate file '%s' - please run '%s' to fix this"),
              "$BASE_DIR/$path", "<b>fusiondirectory-setup</b>"));
      }
    }
  }
  /* Parse configuration file */
  if (session::global_is_set('LOGIN') && session::global_is_set('config') && session::global_is_set('plist')) {
    $config = session::global_get('config');
    $plist  = session::global_get('plist');
    $ui     = session::global_get('ui');
  } else {
    $config = new config(CONFIG_DIR."/".CONFIG_FILE, $BASE_DIR);
    if ($ldap === NULL) {
      $config->set_current(key($config->data['LOCATIONS']));
    } elseif (isset($config->data['LOCATIONS'][$ldap])) {
      $config->set_current($ldap);
    } else {
      authenticateHeader('Invalid LDAP specified');
    }
    session::global_set('DEBUGLEVEL', 0);

    if (($ssl != "") &&
          (($config->get_cfg_value('webserviceForceSSL', 'TRUE') == 'TRUE') ||
           ($config->get_cfg_value("forcessl") == "TRUE"))) {
      echo json_encode(
        array(
          'error' => "HTTP connexions are not allowed, please use HTTPS: $ssl\n"
        )
      );
      exit;
    }

    if (!isset($_SERVER['PHP_AUTH_USER']) && ($user === NULL)) {
      authenticateHeader();
    }
    if ($user !== NULL) {
      $ui = ldap_login_user($user, $pwd);
    } else {
      $ui = ldap_login_user($_SERVER['PHP_AUTH_USER'], $_SERVER['PHP_AUTH_PW']);
    }
    if (!$ui) {
      authenticateHeader('Invalid user or pwd '.$_SERVER['PHP_AUTH_USER'].'/'.$_SERVER['PHP_AUTH_PW']);
    }
    session::global_set('LOGIN', TRUE);
    $plist = new pluglist();
    $config->loadPlist($plist);
    $config->get_departments();
    $config->make_idepartments();
    session::global_set('config', $config);
    session::global_set('plist',  $plist);
    session::global_set('ui',     $ui);
  }
}

/*!
 * \brief This class is the JSON-RPC webservice of FusionDirectory
 *
 * It must be served through jsonRPCServer::handle
 * */
class fdRPCService
{
  public $ldap;

  function __construct ()
  {
  }

  function __call($method, $params)
  {
    global $config;
    if (preg_match('/^_(.*)$/', $method, $m)) {
      throw new Exception("Non existing method '$m[1]'");
    }

    if ($method == 'listLdaps') {
      $config = new config(CONFIG_DIR."/".CONFIG_FILE, $BASE_DIR);
      $ldaps = array();
      foreach ($config->data['LOCATIONS'] as $id => $location) {
        $ldaps[$id] = $location['NAME'];
      }
      return $ldaps;
    } elseif ($method == 'login') {
      /* Login method have the following parameters: LDAP, user, password */
      initiateRPCSession(NULL, array_shift($params), array_shift($params), array_shift($params));
      $method = 'getId';
    } else {
      initiateRPCSession(array_shift($params));
    }

    $this->ldap = $config->get_ldap_link();
    if (!$this->ldap->success()) {
      die('Ldap error: '.$this->ldap->get_error());
    }
    $this->ldap->cd($config->current['BASE']);

    logging::log('debug', 'JSON-RPC', 'Method '.$method, array(), 'Params:'.print_r($params, TRUE));

    return call_user_func_array(array($this, '_'.$method), $params);
  }

  protected function checkAccess($type, $tabs = NULL, $dn = NULL)
  {
    global $ui;
    $infos = objects::infos($type);
    $plist = session::global_get('plist');
    if (($dn !== NULL) && ($ui->dn == $dn)) {
      $self = ':self';
    } else {
      $self = '';
    }
    if (!$plist->check_access($infos['aclCategory'].$self)) {
      throw new Exception("Unsufficient rights for accessing type '$type$self'");
    }
    if ($tabs !== NULL) {
      if (!is_array($tabs)) {
        $tabs = array($tabs);
      }
      foreach ($tabs as $tab) {
        $pInfos = pluglist::pluginInfos($tab);
        if (!$plist->check_access(join($self.',', $pInfos['plCategory']).$self)) {
          throw new Exception("Unsufficient rights for accessing tab '$tab' of type '$type$self'");
        }
      }
    }
  }

  /*!
   * \brief Get list of object of objectType $type in $ou
   */
  protected function _ls ($type, $attrs = NULL, $ou = NULL, $filter = '')
  {
    $this->checkAccess($type);
    return objects::ls($type, $attrs, $ou, $filter);
  }

  /*!
   * \brief Get count of objects of objectType $type in $ou
   */
  protected function _count ($type, $ou = NULL, $filter = '')
  {
    $this->checkAccess($type);
    return objects::count($type, $ou, $filter);
  }

  /*!
   * \brief Get information about objectType $type
   */
  protected function _infos($type)
  {
    $this->checkAccess($type);

    global $config;
    $infos = objects::infos($type);
    unset($infos['tabClass']);
    $infos['tabs'] = $config->data['TABS'][$infos['tabGroup']];
    unset($infos['tabGroup']);
    return $infos;
  }

  /*!
   * \brief List existing object types
   */
  protected function _listTypes()
  {
    global $config;
    $types  = objects::types();

    $result = array();
    foreach ($types as $type) {
      $infos          = objects::infos($type);
      $result[$type]  = $infos['name'];
    }
    return $result;
  }

  /*!
   * \brief List activated tabs on an object
   */
  protected function _listTabs($type, $dn = NULL)
  {
    $this->checkAccess($type, NULL, $dn);

    if ($dn === NULL) {
      $tabobject = objects::create($type);
    } else {
      $tabobject = objects::open($dn, $type);
    }

    $tabs = array();
    foreach ($tabobject->by_object as $tab => $obj) {
      if ($obj->is_account || $obj->ignore_account) {
        $tabs[$tab] = $tabobject->by_name[$tab];
      }
    }
    return $tabs;
  }

  /*!
   * \brief Deactivate a tab of an object
   */
  protected function _removetab($type, $dn, $tab)
  {
    $this->checkAccess($type, $tab, $dn);
    $tabobject = objects::open($dn, $type);
    $tabobject->current = $tab;
    if (!is_subclass_of($tabobject->by_object[$tab], 'simplePlugin')) {
      return array('errors' => array('Tab '.$tab.' is not based on simplePlugin, can’t remove it'));
    } elseif (!$tabobject->by_object[$tab]->displayHeader) {
      return array('errors' => array('Tab '.$tab.' cannot be deactivated, can’t remove it'));
    } elseif (!$tabobject->by_object[$tab]->is_account) {
      return array('errors' => array('Tab '.$tab.' is not activated on '.$dn.', can’t remove it'));
    }
    $_POST = array($tab.'_modify_state' => 1);
    $tabobject->save_object();
    $errors = $tabobject->check();
    if (!empty($errors)) {
      return array('errors' => $errors);
    }
    $tabobject->save();
    return $tabobject->dn;
  }

  /*!
   * \brief Get all form fields from an object (or object type)
   */
  protected function _formfields($type, $dn = NULL, $tab = NULL)
  {
    $this->checkAccess($type, $tab, $dn);

    if ($dn === NULL) {
      $tabobject = objects::create($type);
    } else {
      $tabobject = objects::open($dn, $type);
    }
    if ($tab === NULL) {
      $object = $tabobject->getBaseObject();
    } else {
      $object = $tabobject->by_object[$tab];
    }
    if (is_subclass_of($object, 'simplePlugin')) {
      $fields = $object->attributesInfo;
      foreach ($fields as &$section) {
        $attributes = array();
        foreach ($section['attrs'] as $key => $attr) {
          if ($object->acl_is_readable($attr->getAcl())) {
            $attr->serializeAttribute($attributes, TRUE);
          }
        }
        $section['attrs'] = $attributes;
      }
      unset($section);
      return $fields;
    } else {
      /* fallback for old plugins */
      $fields = array('main' => array('attrs' => array(), 'name' => _('Plugin')));
      foreach ($object->attributes as $attr) {
        if ($object->acl_is_readable($attr.'Acl')) {
          $fields['main']['attrs'][$attr] = array(
            'value'       => $object->$attr,
            'required'    => FALSE,
            'disabled'    => FALSE,
            'label'       => $attr,
            'type'        => 'OldPluginAttribute',
            'description' => '',
          );
        }
      }
      return $fields;
    }
  }

  /*!
   * \brief Update values of an object's attributes using POST as if the webpage was sent
   */
  protected function _formpost($type, $dn, $tab, $values)
  {
    $this->checkAccess($type, $tab, $dn);

    if ($dn === NULL) {
      $tabobject = objects::create($type);
    } else {
      $tabobject = objects::open($dn, $type);
    }
    if ($tab === NULL) {
      $tab = $tabobject->current;
    } else {
      $tabobject->current = $tab;
    }
    $_POST                  = $values;
    $_POST[$tab.'_posted']  = TRUE;
    if (is_subclass_of($tabobject->by_object[$tab], 'simplePlugin') &&
        $tabobject->by_object[$tab]->displayHeader &&
        !$tabobject->by_object[$tab]->is_account
      ) {
      $_POST[$tab.'_modify_state'] = 1;
    }
    $tabobject->save_object();
    $errors = $tabobject->check();
    if (!empty($errors)) {
      return array('errors' => $errors);
    }
    $tabobject->save();
    return $tabobject->dn;
  }

  /*!
   * \brief Get all internal fields from an object (or object type)
   */
  protected function _getfields($type, $dn = NULL, $tab = NULL)
  {
    $this->checkAccess($type, $tab, $dn);

    if ($dn === NULL) {
      $tabobject = objects::create($type);
    } else {
      $tabobject = objects::open($dn, $type);
    }
    if ($tab === NULL) {
      $object = $tabobject->getBaseObject();
    } else {
      $object = $tabobject->by_object[$tab];
    }
    if (is_subclass_of($object, 'simplePlugin')) {
      $fields = $object->attributesInfo;
      foreach ($fields as &$section) {
        $attributes = array();
        foreach ($section['attrs'] as $key => $attr) {
          if ($object->acl_is_readable($attr->getAcl())) {
            $attr->serializeAttribute($attributes, FALSE);
          }
        }
        $section['attrs'] = $attributes;
      }
      unset($section);
    } else {
      /* fallback for old plugins */
      $fields = array('main' => array('attrs' => array(), 'name' => _('Plugin')));
      foreach ($object->attributes as $attr) {
        if ($object->acl_is_readable($attr.'Acl')) {
          $fields['main']['attrs'][$attr] = $object->$attr;
        }
      }
    }
    return $fields;
  }

  /*!
   * \brief Set internal values of an object's attributes and save it
   */
  protected function _setfields($type, $dn, $values)
  {
    $this->checkAccess($type, array_keys($values), $dn);
    if ($dn === NULL) {
      $tabobject = objects::create($type);
    } else {
      $tabobject = objects::open($dn, $type);
    }
    foreach ($values as $tab => $tabvalues) {
      if (is_subclass_of($tabobject->by_object[$tab], 'simplePlugin') &&
          $tabobject->by_object[$tab]->displayHeader &&
          !$tabobject->by_object[$tab]->is_account
        ) {
        if ($tabobject->by_object[$tab]->acl_is_createable()) {
          $tabobject->by_object[$tab]->is_account = TRUE;
        } else {
          return array('errors' => array('You don\'t have sufficient rights to enable tab "'.$tab.'"'));
        }
      }
      $error = $tabobject->by_object[$tab]->deserializeValues($tabvalues);
      if ($error !== TRUE) {
        return array('errors' => array($error));
      }
      $tabobject->current = $tab;
      $tabobject->save_object(); /* Should not do much as POST is empty, but in some cases is needed */
    }
    $errors = $tabobject->check();
    if (!empty($errors)) {
      return array('errors' => $errors);
    }
    $tabobject->save();
    return $tabobject->dn;
  }

  /*!
   * \brief Get all internal fields from a template
   */
  protected function _gettemplate($type, $dn)
  {
    $this->checkAccess($type, NULL, $dn);

    $template = new template($type, $dn);
    return $template->serialize();
  }

  /*!
   * \brief
   */
  protected function _usetemplate($type, $dn, $values)
  {
    $this->checkAccess($type, NULL, $dn);

    $template = new template($type, $dn);
    $error    = $template->deserialize($values);
    if ($error !== TRUE) {
      return array('errors' => array($error));
    }
    $tabobject = $template->apply();
    $errors = $tabobject->check();
    if (!empty($errors)) {
      return array('errors' => $errors);
    }
    $tabobject->save();
    return $tabobject->dn;
  }

  /*!
   * \brief Delete an object
   */
  protected function _delete($type, $dn)
  {
    global $ui;
    $infos = objects::infos($type);
    $plist = session::global_get('plist');
    // Check permissions, are we allowed to remove this object?
    $acl = $ui->get_permissions($dn, $infos['aclCategory'].'/'.$infos['mainTab']);
    if (preg_match('/d/', $acl)) {
      if ($user = get_lock($dn)) {
        return array('errors' => array(sprintf(_('Cannot delete %s. It has been locked by %s.'), $dn, $user)));
      }
      add_lock ($dn, $ui->dn);

      // Delete the object
      $tabobject = objects::open($dn, $type);
      $tabobject->delete();

      // Remove the lock for the current object.
      del_lock($dn);
    } else {
      return array('errors' => array(msgPool::permDelete($dn)));
    }
  }

  /*!
   * \brief Get the session ID
   */
  protected function _getId ()
  {
    return session_id();
  }

  /*!
   * \brief Get the LDAP base
   */
  protected function _getBase ()
  {
    global $config;
    return $config->current['BASE'];
  }
}

$service = new fdRPCService();
if (!jsonRPCServer::handle($service)) {
  echo "no request\n";
  echo session_id()."\n";
}
?>
