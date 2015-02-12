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

class faiTemplateEdit extends plugin
{
  /* attribute list for save action */
  var $config = "";
  var $dn     = "";
  var $value  = "";

  // Encoding identification. Allows to warn the user.
  var $enc_before_edit = "";
  var $enc_after_edit = "";
  var $write_protect = FALSE;
  var $mb_extension = TRUE;

  function faiTemplateEdit (&$config, $dn, $value)
  {
    parent::__construct($config, $dn);
    if(!function_exists("mb_detect_encoding")){
      $this->mb_extension = FALSE;
    }else{
      $this->value = $value;
      $this->enc_before_edit = mb_detect_encoding($value);
      if($this->enc_before_edit != "ASCII"){
        $this->write_protect = TRUE;
      }
    }
  }

  function execute()
  {
    /* Call parent execute */
    parent::execute();

    /* We now split cn/FAItemplatePath to make things more clear... */
    $smarty     = get_smarty();
    $smarty->assign("templateValue",htmlspecialchars(($this->value)));
    $smarty->assign("write_protect",$this->write_protect);
    $smarty->assign("mb_extension",$this->mb_extension);
    return($smarty->fetch(get_template_path('faiTemplateEdit.tpl', TRUE)));
  }

  /* Save data to object */
  function save_object()
  {
    if(!$this->mb_extension) return;
    if(isset($_POST['templateValue']) && !$this->write_protect){
      $this->value = get_post('templateValue');
      $this->enc_after_edit = mb_detect_encoding($this->value);
    }
    if(isset($_POST['editAnyway'])) $this->write_protect = FALSE;
  }


  /* Check supplied data */
  function check()
  {
    $message = array();
    if($this->mb_extension && !$this->write_protect && $this->enc_after_edit !== $this->enc_before_edit ){
      $msg = sprintf(_("The file encodig has changed from '%s' to '%s'. Do you really want to save?"),
        "<i>".$this->enc_before_edit."</i>","<i>".$this->enc_after_edit."</i>");
      $message[] = $msg;
      $this->enc_before_edit = $this->enc_after_edit;
    }
    return($message);
  }

  function save()
  {
    return($this->value);
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
