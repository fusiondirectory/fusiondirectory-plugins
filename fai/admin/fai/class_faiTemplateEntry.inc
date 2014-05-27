<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2015  FusionDirectory

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

class AccessRightsAttribute extends CompositeAttribute
{
  function __construct ($label, $description, $ldapName, $acl = "")
  {
    $attributes = array(
      'u4' => new BooleanAttribute('', '', 'ur'),
      'u2' => new BooleanAttribute('', '', 'uw'),
      'u1' => new BooleanAttribute('', '', 'ue'),
      's4' => new BooleanAttribute('', '', 'us'),
      'g4' => new BooleanAttribute('', '', 'gr'),
      'g2' => new BooleanAttribute('', '', 'gw'),
      'g1' => new BooleanAttribute('', '', 'ge'),
      's2' => new BooleanAttribute('', '', 'gs'),
      'o4' => new BooleanAttribute('', '', 'or'),
      'o2' => new BooleanAttribute('', '', 'ow'),
      'o1' => new BooleanAttribute('', '', 'oe'),
      's1' => new BooleanAttribute('', '', 'os'),
    );
    parent::__construct ($description, $ldapName, $attributes, FALSE, FALSE, $acl, $label);
    $this->setLinearRendering(TRUE);
  }

  function renderFormInput()
  {
    foreach ($this->attributes as &$attribute) {
      $attribute->setDisabled($this->disabled);
    }
    unset($attribute);

    $display = '<table style="border:1px solid #B0B0B0">
         <colgroup width="55" span="3">
          </colgroup>
    <tr>
      <th>{t}Class{/t}</th>
      <th>{t}Read{/t}</th>
      <th>{t}Write{/t}</th>
      <th>{t}Execute{/t}</th>
      <th>&nbsp;</th>
      <th>{t}Special{/t}</th>
      <th>&nbsp;</th>
    </tr>
    <tr><td>{t}User{/t}</td>
      <td align="center">'.$this->attributes['u4']->renderFormInput().'</td>
      <td align="center">'.$this->attributes['u2']->renderFormInput().'</td>
      <td align="center">'.$this->attributes['u1']->renderFormInput().'</td>
      <td>&nbsp;</td>
      <td align="center">'.$this->attributes['s4']->renderFormInput().'</td>
      <td>({t}SUID{/t})</td>
      </tr>

    <tr><td>{t}Group{/t}</td>
      <td align="center">'.$this->attributes['g4']->renderFormInput().'</td>
      <td align="center">'.$this->attributes['g2']->renderFormInput().'</td>
      <td align="center">'.$this->attributes['g1']->renderFormInput().'</td>
      <td>&nbsp;</td>
      <td align="center">'.$this->attributes['s2']->renderFormInput().'</td>
      <td>({t}SGID{/t})</td>
      </tr>

    <tr><td>{t}Others{/t}</td>
      <td align="center">'.$this->attributes['o4']->renderFormInput().'</td>
      <td align="center">'.$this->attributes['o2']->renderFormInput().'</td>
      <td align="center">'.$this->attributes['o1']->renderFormInput().'</td>
      <td>&nbsp;</td>
      <td align="center">'.$this->attributes['s1']->renderFormInput().'</td>
      <td>({t}sticky{/t})</td>
    </tr></table>';

    return $display;
  }

  function readValues($value)
  {
    $tmode  = "$value ";
    $m      = array();
    foreach (array('s', 'u', 'g', 'o') as $type) {
      $current  = substr($tmode, 0, 1);
      $tmode    = preg_replace('/^./', '', $tmode);
      $nr       = 1;
      while ($nr < 5) {
        $m[$type.$nr] = ($current & $nr?'TRUE':'FALSE');
        $nr += $nr;
      }
    }
    $values = array();
    foreach (array_keys($this->attributes) as $name) {
      $values[] = $m[$name];
    }
    return $values;
  }

  function writeValues($values)
  {
    $tmode  = '';
    foreach (array('s', 'u', 'g', 'o') as $type) {
      $nr   = 1;
      $dest = 0;
      while ($nr < 5) {
        if ($values[$type.$nr] === 'TRUE') {
          $dest += $nr;
        }
        $nr += $nr;
      }
      $tmode = $tmode.$dest;
    }
    return $tmode;
  }
}

class faiTemplateEntry extends simplePlugin
{
  var $mainTab        = TRUE;
  var $objectclasses  = array('top', 'FAIclass', 'FAItemplateEntry');

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Properties'),
        'attrs' => array(
          new HostNameAttribute (
            _('File name'), _('Template file name'),
            'cn', TRUE
          ),
          new StringAttribute (
            _('Description'), _('Short description of the file'),
            'description', FALSE
          ),
          new StringAttribute (
            _('File path'), _('Where the file will be stored'),
            'FAItemplatePath', FALSE
          ),
        )
      ),
      'template' => array(
        'name'  => _('Template attributes'),
        'class' => array('fullwidth'),
        'attrs' => array(
          new FileTextAreaAttribute (
            _('Template file content'), _('Content of the template file'),
            'FAItemplateFile'
          ),
          new CompositeAttribute (
            _('Owner and group of the file'), 'FAIowner',
            array(
              new StringAttribute (
                _('Owner'), _('File owner'),
                'owner', TRUE,
                'root'
              ),
              new StringAttribute (
                _('Group'), _('File group'),
                'group', TRUE,
                'root'
              ),
            ),
            '/^([^.]+)\.([^.]+)$/',
            '%s.%s'
          ),
          new AccessRightsAttribute (
            _('Access'), _('Access rights'),
            'FAImode'
          )
        )
      )
    );
  }
}

class faiTemplateEntry_old extends plugin
{
  /* attribute list for save action */
  var $ignore_account= TRUE;
  var $attributes   = array("cn","description","FAItemplateFile","FAItemplatePath","FAImode","user","group","binary","FAIowner");
  var $objectclasses= array();

  var $orig_cn              = "";

  var $dn            = "";
  var $cn            = "";
  var $FAItemplateFile   = "";
  var $FAItemplatePath   = "";
  var $description   = "";
  var $status        = "new";
  var $FAImode       = "0640";
  var $FAIowner      = "root.root";
  var $user          = "root";
  var $group         = "root";
  var $binary        = false;
  var $parent        = NULL;
  var $FAIstate      = "";

  function faiTemplateEntry (&$config, $dn= NULL,$object=false)
  {
    parent::__construct($config, $dn);

    if((isset($object['cn'])) && (!empty($object['cn']))){
      $this->orig_cn= $object['cn'];
      $this->dn=$object['dn'];
      foreach($object as $name=>$value){
        $oname = $name;
        $this->$oname=$value;
      }
    }else{
      $this->status = "new";
      $this->orig_cn= false;
    }

    $this->user = explode( '.', $this->FAIowner );
    $this->group = $this->user[1];
    $this->user = $this->user[0];

    session::set('binary',$this->FAItemplateFile);
    session::set('binarytype','octet-stream');
    session::set('binaryfile',basename($this->FAItemplatePath));

    if(!empty($this->dn) && $this->dn != "new"){
      $ldap = $this->config->get_ldap_link();
      session::set('binary',$ldap->get_attribute($this->dn,"FAItemplateFile"));
      $this->FAItemplateFile  = session::get('binary');
    }

    $this->FAImode= sprintf("%0.4s", $this->FAImode)." ";
  }


  function execute()
  {
    /* Call parent execute */
    parent::execute();

    /* Fill templating stuff */
    $smarty     = get_smarty();
    $smarty->assign("rand", rand(0, 10000));
    $display = "";

    if(isset($_POST['TmpFileUpload']) && $this->acl_is_writeable("FAItemplateFile")){
      if($str=file_get_contents($_FILES['FAItemplateFile']['tmp_name'])){
        $str = preg_replace("/[\r]/","",$str);
        $this->FAItemplateFile = $str;

        /* If we don't have a filename set it from upload filename. */
        if( 0 == strlen(preg_replace("/^.*\//","",$this->FAItemplatePath))){
          $this->FAItemplatePath = preg_replace("/[^\/]*$/","",$this->FAItemplatePath).$_FILES['FAItemplateFile']['name'];
        }

        session::set('binary',$this->FAItemplateFile);
        session::set('binarytype','octet-stream');
        session::set('binaryfile',basename($this->FAItemplatePath));
      }
    }

    /* File download requested */
    if(isset($_GET['getFAItemplate'])){
      send_binary_content($this->FAItemplateFile,$this->cn.".FAItemplate");
    }

    /* File edit requested */
    if(isset($_GET['editFAItemplate'])){
      $this->dialog = new faiTemplateEdit($this->config,$this->dn,$this->FAItemplateFile);
    }

    /* File edit requested, was canceled  */
    if(isset($_POST['templateEditCancel'])){
      $this->dialog = null;
    }

    /* File edit requested, was canceled  */
    if($this->dialog instanceOf faiTemplateEdit && isset($_POST['templateEditSave'])){
      $this->dialog->save_object();
      $msgs = $this->dialog->check();
      if(count($msgs)){
        msg_dialog::displayChecks($msgs);
      }else{
        $this->FAItemplateFile = $this->dialog->save();
        $this->dialog = null;
      }
    }

    /* Display opened dialog */
    if($this->dialog){
      $this->dialog->save_object();
      return($this->dialog->execute());
    }

    $status= _("no file uploaded yet");
    $bStatus = false; // Hide download icon on default
    if(strlen($this->FAItemplateFile)){
      $status= sprintf(_("exists in database (size: %s bytes)"),strlen($this->FAItemplateFile));
      $bStatus = true;  // Display download icon
    }

    $smarty->assign("status",$status);
    $smarty->assign("bStatus",$bStatus);

    foreach($this->attributes as $attrs){
      $smarty->assign($attrs,($this->$attrs));
    }

    /* Assign file modes */
    $tmode= "$this->FAImode ";
    foreach (array("s", "u", "g", "o") as $type){
      $current= substr($tmode, 0, 1);
      $tmode=   preg_replace("/^./", "", $tmode);
      $nr= 1;
      while ($nr < 5){
        if ($current & $nr){
          $smarty->assign($type.$nr, "checked");
        } else {
          $smarty->assign($type.$nr, "");
        }
        $nr+= $nr;
      }
    }

    foreach($this->attributes as $attr){
      $smarty->assign($attr."ACL",$this->getacl($attr,preg_match("/freeze/",$this->FAIstate)));
    }

    /* We now split cn/FAItemplatePath to make things more clear... */
    $smarty->assign("templateFile", preg_replace("/^.*\//","",$this->FAItemplatePath));
    $smarty->assign("templatePath", preg_replace("/[^\/]*$/","",$this->FAItemplatePath));
    $smarty->assign("freeze", preg_match("/freeze/i",$this->FAIstate));;

    $display.=  $smarty->fetch(get_template_path('faiTemplateEntry.tpl', TRUE));
    return($display);
  }

  /* Save data to object */
  function save_object()
  {
    /* Check if form is posted and we are not freezed */
    if((isset($_POST['SubObjectFormSubmitted'])) && !preg_match("/freeze/", $this->FAIstate)){

      /* Remember destination current path
          depending on the ACLs we will assemble a new one later.
       */
      $cur_path = $this->FAItemplatePath;
      parent::save_object();

      /* Set user.group (FAIowner) attribute */
      if(isset($_POST['group']) && isset($_POST["user"]) && $this->acl_is_writeable("FAIowner")){
        $this->FAIowner = $_POST["user"].'.'.$_POST["group"];
        $this->user = $_POST['user'];
        $this->group= $_POST['group'];
      }

      /* Check if permissions have changed */
      if($this->acl_is_writeable("FAImode")){

        /* Save mode */
        $tmode= "";
        foreach (array("s", "u", "g", "o") as $type){
          $nr= 1;
          $dest= 0;
          while ($nr < 5){
            if (isset($_POST["$type$nr"])){
              $dest+= $nr;
            }
            $nr+= $nr;
          }
          $tmode= $tmode.$dest;
        }
        $this->FAImode= $tmode;
      }

      /* Check if we are allowed to change the destination directory
       */
      if($this->acl_is_writeable("FAItemplatePath")){
        $cur_path = get_post('templatePath').'/'.basename($cur_path);
      }

      /* Check if we are allowed to change the destination directory
       */
      if($this->acl_is_writeable("cn")){
        $cur_path = preg_replace("/[^\/]*$/","",$cur_path).trim(get_post('templateFile'));
      }
      $cur_path= str_replace("//","/",$cur_path);
      if(trim($cur_path,"/") == ""){
        $cur_path= "";
      }
      $this->FAItemplatePath = $this->cn= $cur_path;
    }
  }


  /* Check supplied data */
  function check()
  {
    /* Call common method to give check the hook */
    $message= plugin::check();

    if(isset($this->parent->SubObjects[$this->cn]) && $this->cn != $this->orig_cn){
      $message[] = msgPool::duplicated(_("Name"));
    }

    if(empty($this->FAItemplateFile)){
      $message[]= msgPool::required(_("File"));
    }

    if(!preg_match('/^\//', $this->FAItemplatePath)){
      $message[]= msgPool::invalid(_("Destination path"),"","","/path");
    }

    $b = trim(basename($this->FAItemplatePath));
    if($b == ""){
      $message[] = msgPool::required(_("File name"));
    }

    if($this->user == ""){
      $message[] = msgPool::required(_("User"));
    }elseif(preg_match("/[^0-9a-z]/i",$this->user)){
      $message[] = msgPool::invalid(_("User"),$this->user,"/[0-9a-z]/");
    }

    if($this->group == ""){
      $message[] = msgPool::required(_("Group"));
    }elseif (!tests::is_uid($this->group)){
      if (strict_uid_mode()){
        $message[]= msgPool::invalid(_("Group"), $this->group, "/[a-z0-9_-]/");
      } else {
        $message[]= msgPool::invalid(_("Group"), $this->group, "/[a-z0-9_-]/i");
      }
    }

    return ($message);
  }

  function save()
  {
    $tmp=array();
    foreach($this->attributes as $attrs){
      $tmp[$attrs] = $this->$attrs;
    }

    if(($this->orig_cn)&&($tmp['cn']!=$this->orig_cn)){
      $tmp['remove']['from']  = $this->orig_cn;
      $tmp['remove']['to']    = $tmp['cn'];
    }

    $tmp['dn']      = $this->dn;
    $tmp['status']  = $this->status;

    return($tmp);
  }
}
?>
