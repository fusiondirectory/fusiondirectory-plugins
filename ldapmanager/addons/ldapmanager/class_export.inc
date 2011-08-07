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
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/

class ldifexport extends plugin
{
    /* Definitions */
    var $plHeadline= "LDIF export";
    var $plDescription= "This plugin allow the export of the ldap tree in ldif format";

    /* attribute list for save action */
    var $attributes= array();
    var $objectclasses= array();
    var $view_logged = FALSE;

    var $base;

    function ldifexport (&$config, $dn= NULL)
    {
        /* Include config object */
        $this->config= &$config;
        $this->ui = get_userinfo();

        $this->baseSelector= new baseSelector($this->config->idepartments, $this->config->current['BASE']);
        $this->baseSelector->setSubmitButton(false);
        $this->baseSelector->setHeight(300);
        $this->baseSelector->update(true);

    }

    function execute()
    {
        /* Call parent execute */
        plugin::execute();

        /* Log view */
        if(!$this->view_logged){
            $this->view_logged = TRUE;
            new log("view","all/".get_class($this),$this->dn);
        }

        // Update baseSelector
        if (!$this->baseSelector->update()) {
            msg_dialog::display(_("Error"), msgPool::permMove(), ERROR_DIALOG);
        }
        if ($this->base != $this->baseSelector->getBase()) {
            $this->base= $this->baseSelector->getBase();
            $this->is_modified= TRUE;
        }


        // Assign smarty values 
        $smarty= get_smarty();

        $smarty->assign("single","");
        $smarty->assign("base", $this->baseSelector->render());

        // fill in old vars in the Export form
        if(isset($_POST['single'])) $smarty->assign("single",get_post('single'));

        // Get the LDAP link, to generate the Export
        $ldap = $this->config->get_ldap_link();
        $dn = "";
        if(isset($_POST['sfrmgetsingle']))
        {  
            $dn   = get_post('single');
            $scope= "base";
            $acls = $this->ui->get_permissions($dn,"ldapmanager/ldif");
            $name = "entryExport.ldif";
            if(!$ldap->dn_exists($dn)){
                msg_dialog::display(_("LDAP error"),
                        sprintf(_("No such object %s!"),LDAP::fix($dn)),
                        ERROR_DIALOG);
                $dn ="";
            }

        }elseif(isset($_POST['sfrmgetfull'])){
            $dn   = $this->base;
            $scope= "sub";
            $acls = $this->ui->get_permissions($dn,"ldapmanager/ldif");
            $name = "fullExport.ldif";
        }

        if(!empty($dn)){

            // Check permissions before sending the output
            if(!preg_match("/r/",$acls)){
                msg_dialog::display(_("Permission error"), 
                        sprintf(_("You have no permission to export %s!"), bold(LDAP::fix($dn))),
                        ERROR_DIALOG);
            }else{
                $data = $ldap->generateLdif(LDAP::fix($dn), 'objectClass=*',array(),$scope, $limit=0);
                if($data === NULL){
                    msg_dialog::display(_("Error"), 
                            sprintf(_("Failed to generate ldap export, error was '%s'!"), $ldap->get_error()),
                            ERROR_DIALOG);
                }else{
                    send_binary_content($data,$name);
                }
            }
        }

        // Show the main form
        return ($smarty->fetch (get_template_path('contentexport.tpl', TRUE)));
    }
}


// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
