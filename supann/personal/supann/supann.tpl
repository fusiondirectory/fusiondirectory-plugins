<script language="javascript" src="include/overlib.js" type="text/javascript"></script>
<table style="text-align: left; width: 100%;" border="0" cellpadding="2" cellspacing="2">
  <tbody>
    <tr>
      <td colspan="2" style="vertical-align: top; width: 49%;">
        <h1><img alt="" align="middle" src="plugins/supann/images/profil.png" class="center"> {t}Common Profil{/t}</h1>
      </td>
        <td style="width: 2%">
        </td>

      <td>
        <h2>{t}Additional Profil{/t}</h2>
      </td>
      <td>
        <select size="1" id="type_profil" name="type_profil"  onChange="document.mainform.submit();">
          {html_options options=$profil_list selected=$type_profil}
        </select>
        profil choisi =  {$type_profil}
      </td>

    </tr>
    <tr>
      <td colspan="5" style="vertical-align: top;border-top: 1px solid rgb(160, 160, 160);border-left: 1px solid rgb(160, 160, 160);">
        <h2>{t}Identity{/t}</h2>
      </td>
    </tr>
    <tr> <!-- supannCivilite - eduPersonPrincipalName -->
      <td>
        <label for="supannCivilite">{t}supannCivilite{/t}</label>
      </td>
      <td>
        <select size="1" id="supannCivilite" name="supannCivilite" onChange="document.mainform.submit();">
          {html_options values=$list_supannCivilite output=$list_supannCivilite selected=$supannCivilite}
        </select>
      </td>
      <!-- vertical separator -->
      <td rowspan="2" style="border-left: 1px solid rgb(160, 160, 160);">
        &nbsp;
      </td>

      <td>
        <label for="eduPersonPrincipalName">{t}eduPersonPrincipalName{/t}</label>
      </td>
      <td>
        <input name="eduPersonPrincipalName" id="eduPersonPrincipalName" value="{$eduPersonPrincipalName}" size="35">
      </td>
    </tr>
    <tr> <!-- supannAliasLogin - eduPersonNickName -->
      <td>
        <label for="supannAliasLogin">{t}supannAliasLogin{/t}</label>
      </td>
      <td>
        <input name="supannAliasLogin" id="supannAliasLogin" value="{$supannAliasLogin}" size="35">
      </td>
      <!-- vertical separator -->


      <td>
        <label for="eduPersonNickName">{t}eduPersonNickName{/t}</label>
      </td>
      <td>
        <input name="eduPersonNickName" id="eduPersonNickName" value="{$eduPersonNickName}" size="35">
      </td>
    </tr>

    <tr>
      <td colspan="5" style="width: 50%; vertical-align: top;border-top: 1px solid rgb(160, 160, 160);border-left: 1px solid rgb(160, 160, 160);">
        <h2>{t}Contact{/t}</h2>
      </td>
    </tr>
    <tr> <!-- ENTETE supannAutreMail - supannMailPerso -->
      <td colspan="2">
        <label for="supannAutreMail_new">{t}supannAutreMail{/t}</label>
      </td>
      <!-- vertical separator -->
      <td rowspan="6" style="border-left: 1px solid rgb(160, 160, 160);">
        &nbsp;
      </td>

      <td colspan="2">
        <label for="supannMailPerso_new">{t}supannMailPerso{/t}</label>
      </td>
    </tr>
    <tr> <!-- DONNEE supannAutreMail - supannMailPerso -->
      <td colspan="2">
        <select name="supannAutreMail_selected[]" id="supannAutreMail_selected" multiple="multiple" size="2" style="width: 100%;">
          {html_options options=$supannAutreMail_values}
        </select>
      </td>

      <td colspan="2">
        <select name="supannMailPerso_selected[]" id="supannMailPerso_selected" multiple="multiple" size="2" style="width: 100%;">
          {html_options options=$supannMailPerso_values}
        </select>
      </td>
    </tr>
    <tr> <!-- SAISIE supannAutreMail - supannMailPerso -->
      <td colspan="2">
        <input id="supannAutreMail_new" name="supannAutreMail_new" size="30" maxlength="100" type="text">
        <input name="addsupannAutreMail" value="{msgPool type=addButton}" id="addsupannAutreMail" type="submit">
        <input name="delsupannAutreMail" value="{msgPool type=delButton}" id="delsupannAutreMail" type="submit">
      </td>

      <td colspan="2">
        <input id="supannMailPerso_new" name="supannMailPerso_new" size="10" maxlength="50" type="text">
        <input name="addsupannMailPerso" value="{msgPool type=addButton}" id="addsupannMailPerso" type="submit">
        <input name="delsupannMailPerso" value="{msgPool type=delButton}" id="delsupannMailPerso" type="submit">
      </td>
    </tr>
    <tr><!-- ENTETE supannAutreTelephone - supannListeRouge -->
      <td colspan="2">
        <label for="supannAutreTelephone_new">{t}supannAutreTelephone{/t}</label>
      </td>
      <td colspan="2">
        <label for="supannListeRouge">{t}supannListeRouge{/t}</label>
      </td>
    </tr>
    <tr><!-- DONNEE supannAutreTelephone - supannListeRouge -->
      <td colspan="2">
        <select name="supannAutreTelephone_selected[]" id="supannAutreTelephone_selected" multiple="multiple" size="2" style="width: 100%;">
          {html_options options=$supannAutreTelephone_values}
        </select>
      </td>


      <td colspan="2">
        <input type="checkbox" name="supannListeRouge" id="supannListeRouge" value="TRUE" {if $supannListeRouge == "TRUE"}checked{/if}>
      </td>
    </tr>
    <tr><!-- SAISIE supannAutreTelephone - supannListeRouge -->
      <td colspan="2">
        <input name="supannAutreTelephone_new" id="supannAutreTelephone_new" size="14" type="text">
        <input name="addsupannAutreTelephone" value="{msgPool type=addButton}" id="addsupannAutreTelephone" type="submit">
        <input name="delsupannAutreTelephone" value="{msgPool type=delButton}" id="delsupannAutreTelephone" type="submit">
      </td>

      <td colspan="2">
        <!-- LIBRE -->
      </td>
    </tr>
    </tbody>
    </table>
    <p class="seperator">&nbsp;</p>
    <table>
    <tbody>
    <tr>
      <td colspan="5">
        <h1><img alt="" align="middle" src="plugins/supann/images/affiliation.png" class="center"> {t}Affectation{/t} - {t}Affiliation{/t}</h1>
      </td>
    </tr>
    <tr> <!-- ENTETE AFFECTATION - AFFILIATION -->
      <td colspan="2" style="width: 50%;vertical-align: top;border-top: 1px solid rgb(160, 160, 160);border-left: 1px solid rgb(160, 160, 160);">
        <h2>{t}Affectation{/t}</h2>
      </td>
      <!-- vertical separator -->
      <td rowspan="8" style="border-left: 1px solid rgb(160, 160, 160);">
        &nbsp;
      </td>
      <td colspan="2" style="vertical-align: top;border-top: 1px solid rgb(160, 160, 160);border-left: 1px solid rgb(160, 160, 160);">
        <h2>{t}Affiliation{/t}</h2>
      </td>
    </tr>
    <tr> <!-- ENTETE supannEntiteAffectationPrincipale - eduPersonPrimaryAffiliation -->
      <td> <!-- ENTETE  supannEntiteAffectationPrincipale-->
        <label for="supannEntiteAffectationPrincipale">{t}supannEntiteAffectationPrincipale{/t}</label>
      </td>
      <td> <!-- SAISIE/DONNE  supannEntiteAffectationPrincipale-->
        <select name="supannEntiteAffectationPrincipale" id="supannEntiteAffectationPrincipale"  >
          {html_options options=$list_supannEntiteAffectation selected=$supannEntiteAffectationPrincipale }
        </select>
      </td>
      <td> <!-- ENTETE  eduPersonPrimaryAffiliation-->
        <label for="eduPersonPrimaryAffiliation">{t}eduPersonPrimaryAffiliation{/t}</label>
      </td>
      <td> <!-- SAISIE/DONNE  eduPersonPrimaryAffiliation-->
        <select name="eduPersonPrimaryAffiliation" id="eduPersonPrimaryAffiliation"  >
          {html_options options=$list_eduPersonAffiliation selected=$eduPersonPrimaryAffiliation}
        </select>
      </td>
    </tr>
    <tr><!-- ENTETE supannEntiteAffectation - eduPersonAffiliation -->
      <td colspan="2"><!-- ENTETE supannEntiteAffectation -->
        <label for="supannEntiteAffectation_new">{t}supannEntiteAffectation{/t}</label>
      </td>
      <td colspan="2"><!-- eduPersonAffiliation -->
        <label for="eduPersonAffiliation_new">{t}eduPersonAffiliation{/t}</label>
      </td>
    </tr>
    <tr><!-- DONNE supannEntiteAffectation - eduPersonAffiliation -->
      <td colspan="2"><!-- DONNEE supannEntiteAffectation -->
        <select name="supannEntiteAffectation_selected[]" id="supannEntiteAffectation_selected" multiple="multiple" size="2" style="width: 100%;">
          {html_options options=$supannEntiteAffectation_values}
        </select>
      </td>
      <td colspan="2"><!-- DONNEE  eduPersonAffiliation -->
        <select name="eduPersonAffiliation_selected[]" id="eduPersonAffiliation_selected" multiple="multiple" size="2" style="width: 100%;">
          {html_options options=$eduPersonAffiliation_values}
        </select>
      </td>
    </tr>
    <tr><!-- SAISIE supannEntiteAffectation - eduPersonAffiliation -->
      <td colspan="2"><!-- SAISIE supannEntiteAffectation -->
        <select name="supannEntiteAffectation_new" id="supannEntiteAffectation_new"  >
          {html_options options=$list_supannEntiteAffectation }
        </select>
        <input name="addsupannEntiteAffectation" value="{msgPool type=addButton}" id="addsupannEntiteAffectation" type="submit">
        <input name="delsupannEntiteAffectation" value="{msgPool type=delButton}" id="delsupannEntiteAffectation" type="submit">
      </td>
      <td colspan="2"><!-- SAISIE  eduPersonAffiliation -->
        <!-- lmist des affiliation possible droplist-->
        <select name="eduPersonAffiliation_new" id="eduPersonAffiliation_new"  >
          {html_options options=$list_eduPersonAffiliation}
        </select>
        <input name="addeduPersonAffiliation" value="{msgPool type=addButton}" id="addeduPersonAffiliation" type="submit">
        <input name="deleduPersonAffiliation" value="{msgPool type=delButton}" id="deleduPersonAffiliation" type="submit">
      </td>
    </tr>
    <tr><!-- ENTETE supannTypeEntiteAffectation - supannEtablissement -->
      <td colspan="2"><!-- ENTETE supannTypeEntiteAffectation -->
        <label for="supannTypeEntiteAffectation_new">{t}supannTypeEntiteAffectation{/t}</label>
      </td>
      <td colspan="2"><!-- supannEtablissement -->
        <label for="supannEtablissement_new">{t}supannEtablissement{/t}</label>
      </td>
    </tr>
    <tr><!-- DONNE supannTypeEntiteAffectation - supannEtablissement -->
      <td colspan="2"><!-- DONNEE supannTypeEntiteAffectation -->
        <select name="supannTypeEntiteAffectation_selected[]" id="supannTypeEntiteAffectation_selected" multiple="multiple" size="2" style="width: 100%;">
          {html_options options=$supannTypeEntiteAffectation_values}
        </select>
      </td>
      <td colspan="2"><!-- DONNEE  supannEtablissement -->
        <select name="supannEtablissement_selected[]" id="supannEtablissement_selected" multiple="multiple" size="2" style="width: 100%;">
          {html_options options=$supannEtablissement_values}
        </select>
      </td>
    </tr>
    <tr><!-- SAISIE supannTypeEntiteAffectation - supannEtablissement -->
      <td colspan="2"><!-- SAISIE supannTypeEntiteAffectation -->
        <select name="ref_supannTypeEntiteAffectation" id="ref_supannTypeEntiteAffectation" onChange="document.mainform.submit();">
          {html_options values=$list_ref_supannTypeEntiteAffectation output=$list_ref_supannTypeEntiteAffectation selected=$ref_supannTypeEntiteAffectation}
        </select>
        <select name="supannTypeEntiteAffectation_new" id="supannTypeEntiteAffectation_new"  >
          {html_options options=$list_supannTypeEntiteAffectation}
        </select>
        <input name="addsupannTypeEntiteAffectation" value="{msgPool type=addButton}" id="addsupannTypeEntiteAffectation" type="submit">
        <input name="delsupannTypeEntiteAffectation" value="{msgPool type=delButton}" id="delsupannTypeEntiteAffectation" type="submit">
      </td>
      <td colspan="2"><!-- SAISIE  supannEtablissement -->
        <select name="ref_supannEtablissement" id="ref_supannEtablissement"  onChange="document.mainform.submit();">
          {html_options values=$list_ref_supannEtablissement output=$list_ref_supannEtablissement selected=$ref_supannEtablissement}
        </select>
        <select name="supannEtablissement_new" id="supannEtablissement_new"  >
          {html_options options=$list_supannEtablissement}
        </select>
        <input name="addsupannEtablissement" value="{msgPool type=addButton}" id="addsupannEtablissement" type="submit">
        <input name="delsupannEtablissement" value="{msgPool type=delButton}" id="delsupannEtablissement" type="submit">
      </td>
    </tr>
 </tbody>
</table>



{if $type_profil == "personal"}
<table style="text-align: left; width: 100%;" border="0" cellpadding="2" cellspacing="2">
  <tbody>
    <tr>
      <td colspan="5" style="vertical-align: top;border-top: 1px solid rgb(160, 160, 160);border-left: 1px solid rgb(160, 160, 160);">
        <h1><img alt="" align="middle" src="plugins/supann/images/user-employee.png" class="center"> {t}Personal Profil{/t}</h1>
      </td>
    </tr>
    <tr> <!-- ENTETE supannEmpId - supannActivite -->
      <td colspan="2" style="vertical-align: top;width: 49%">
        <label for="supannEmpId">{t}supannEmpId{/t}</label>
      </td>
      <!-- vertical separator -->
      <td rowspan="6" style="border-left: 1px solid rgb(160, 160, 160);">
        &nbsp;
      </td>

      <td colspan="2">
        <label for="supannActivite_new">{t}supannActivite{/t}</label>
      </td>
    </tr>
    <tr> <!-- DONNEES supannEmpId - supannActivite -->
      <td colspan="2">
        <input name="supannEmpId" id="supannEmpId" value="{$supannEmpId}" size="35">
      </td>
      <td colspan="2">
        <select name="supannActivite_selected[]" id="supannActivite_selected" multiple="multiple" size="2" style="width: 100%;">
          {html_options options=$supannActivite_values}
        </select>
      </td>
    </tr>
    <tr> <!-- SAISIES supannEmpId - supannActivite -->
      <td colspan="2">
        <!--FREE CASE -->
      </td>
      <td colspan="2">
        <select name="ref_supannActivite" id="ref_supannActivite"  onChange="document.mainform.submit();">
          {html_options values=$list_ref_supannActivite output=$list_ref_supannActivite selected=$ref_supannActivite}
        </select>
        <select name="supannActivite_new" id="supannActivite_new"  >
          {html_options options=$list_supannActivite}
        </select>

        <input name="addsupannActivite" value="{msgPool type=addButton}" id="addsupannActivite" type="submit">
        <input name="delsupannActivite" value="{msgPool type=delButton}" id="delsupannActivite" type="submit">
      </td>
    </tr>
    <tr> <!-- ENTETE supannEmpCorps - supannRoleGenerique -->
      <td colspan="2">
        <label for="supannEmpCorps_new">{t}supannEmpCorps{/t}</label>
      </td>
      <td colspan="2">
        <label>{t}supannRoleGenerique{/t}</label>
      </td>
    </tr>
    <tr> <!-- DONNEES supannEmpCorps - supannRoleGenerique -->
      <td colspan="2">
        <select name="supannEmpCorps_selected[]" id="supannEmpCorps_selected" multiple="multiple" size="2" style="width: 100%;">
          {html_options options=$supannEmpCorps_values}
        </select>
      </td>
      <td colspan="2">
        <select name="supannRoleGenerique_selected[]" id="supannRoleGenerique_selected" multiple="multiple" size="2" style="width: 100%;">
          {html_options options=$supannRoleGenerique_values}
        </select>
      </td>
    </tr>
    <tr> <!-- SAISIE supannEmpCorps - supannRoleGenerique -->
      <td colspan="2">
        <select name="ref_supannEmpCorps" id="ref_supannEmpCorps"   onChange="document.mainform.submit();" >
          {html_options values=$list_ref_supannEmpCorps output=$list_ref_supannEmpCorps selected=$ref_supannEmpCorps}
        </select>
        <select name="supannEmpCorps_new" id="supannEmpCorps_new"  >
          {html_options options=$list_supannEmpCorps}
        </select>
        <input name="addsupannEmpCorps" value="{msgPool type=addButton}" id="addsupannEmpCorps" type="submit">
        <input name="delsupannEmpCorps" value="{msgPool type=delButton}" id="delsupannEmpCorps" type="submit">
      </td>
      <td colspan="2">
      </td>
    </tr>
    <tr> <!-- ENTETE supannRoleEntite -->
      <td colspan="5">
        <label for="ref_supannRoleGenerique_SRE">{t}supannRoleEntite{/t}</label>
      </td>
    </tr>
    <tr> <!-- DONNEES supannRoleEntite -->
      <td colspan="5">
        {$supannEntiteRoleList}
      </td>
    </tr>
    <tr> <!-- SAISIES supannRoleEntite -->
      <td colspan="5">
        <select name="ref_supannRoleGenerique_SRE" id="ref_supannRoleGenerique_SRE"  onChange="document.mainform.submit();" >
          {html_options values=$list_ref_supannRoleGenerique_SRE output=$list_ref_supannRoleGenerique_SRE selected=$ref_supannRoleGenerique_SRE}
        </select>
        <select name="supannRoleGenerique_SRE" id="supannRoleGenerique_SRE"  >
          {html_options options=$list_supannRoleGenerique_SRE }
        </select>
        <select name="ref_supannTypeEntiteAffectation_SRE" id="ref_supannTypeEntiteAffectation_SRE"  onChange="document.mainform.submit();" >
          {html_options values=$list_ref_supannTypeEntiteAffectation_SRE output=$list_ref_supannTypeEntiteAffectation_SRE selected=$ref_supannTypeEntiteAffectation_SRE}
        </select>
        <select name="supannTypeEntiteAffectation_SRE" id="supannTypeEntiteAffectation_SRE"  >
          {html_options options=$list_supannTypeEntiteAffectation_SRE}
        </select>
        <select name="supannEntiteAffectation_SRE" id="supannEntiteAffectation_SRE"  >
          {html_options options=$list_supannEntiteAffectation}
        </select>
        <input name="addsupannRoleEntite" value="{msgPool type=addButton}" id="addsupannRoleEntite" type="submit">
      </td>
    </tr>

  </tbody>
</table>
{/if}
{if $type_profil == "student"}
<table style="text-align: left; width: 100%;" border="0" cellpadding="2" cellspacing="2">
  <tbody>
    <tr>
      <td colspan="5">
        <h1><img alt="" align="middle" src="plugins/supann/images/user-student.png" class="center"> {t}Student Profil{/t}</h1>
      </td>
    </tr>
    <tr> <!-- INTITULE supannCodeINE supannEtuId -->
      <td colspan="2" style="vertical-align: top;width: 50%;">
        <label for="supannCodeINE_new">{t}supannCodeINE{/t}</label>
      </td>
                  <!-- vertical separator -->
      <td rowspan="3" style="border-left: 1px solid rgb(160, 160, 160);">
        &nbsp;
      </td>
      <td colspan="2">
        <label for="supannEtuId_new">{t}supannEtuId{/t}</label>
      </td>
    </tr>
    <tr><!-- DONNEES supannCodeINE supannEtuId -->
      <td colspan="2" style="vertical-align: top;" >
        <select name="supannCodeINE_selected[]" id="supannCodeINE_selected" multiple="multiple" size="2" style="width: 100%;">
          {html_options options=$supannCodeINE_values}
        </select>
      </td>
      <td colspan="2" style="vertical-align: top;">
        <select name="supannEtuId_selected[]" id="supannEtuId_selected" multiple="multiple" size="2" style="width: 100%;">
          {html_options options=$supannEtuId_values}
        </select>
      </td>
    </tr>
    <tr><!-- SAISIE supannCodeINE supannEtuId -->
      <td colspan="2">
        <input id="supannCodeINE_new" name="supannCodeINE_new" size="10" maxlength="50" type="text">
        <input name="addsupannCodeINE" value="{msgPool type=addButton}" id="addsupannCodeINE" type="submit">
        <input name="delsupannCodeINE" value="{msgPool type=delButton}" id="delsupannCodeINE" type="submit">
      </td>
      <td colspan="2">
        <input id="supannEtuId_new" name="supannEtuId_new" size="10" maxlength="50" type="text">
        <input name="addsupannEtuId" value="{msgPool type=addButton}" id="addsupannEtuId" type="submit">
        <input name="delsupannEtuId" value="{msgPool type=delButton}" id="delsupannEtuId" type="submit">
      </td>
    </tr>

<!-- ETU INSCRIPTION -->
    <tr><!-- INTITULE supannEtuInscription supannEtuEtape -->
      <td colspan="5" style="vertical-align: top;border-top: 1px solid rgb(160, 160, 160);border-left: 1px solid rgb(160, 160, 160);">
        <h2>{t}Student subscription{/t}</h2>
      </td>
    </tr>
    <tr><!-- INTITULE supannEtuInscription -->
      <td colspan="5">
        <label for="ref_supannEtablissement_SEI">{t}supannEtuInscription{/t}</label>
      </td>
    </tr>
    <tr><!-- DONNES supannEtuInscription -->
      <td colspan="5">
        {$supannEtuInscriptionList}
      </td>
    </tr>
    <tr><!-- saisie   RANG 1 supannEtablissement  supannEtuDiplome -->
      <td>
        <label for="ref_supannEtablissement_SEI">{t}supannEtablissement{/t}</label>
      </td>
      <td>
        <select name="ref_supannEtablissement_SEI" id="ref_supannEtablissement_SEI"  onChange="document.mainform.submit();">
          {html_options values=$list_ref_supannEtablissement_SEI output=$list_ref_supannEtablissement_SEI selected=$ref_supannEtablissement_SEI}
        </select>
        <select name="supannEtablissement_SEI" id="supannEtablissement_SEI"  >
          {html_options options=$list_supannEtablissement_SEI selected=$supannEtablissement_SEI}
        </select>
      </td>
      <td></td>
      <td>
        <label for="ref_supannEtuTypeDiplome_SEI">{t}supannEtuTypeDiplome{/t}</label>
      </td>
      <td>
        <select name="ref_supannEtuTypeDiplome_SEI" id="ref_supannEtuTypeDiplome_SEI"  onChange="document.mainform.submit();">
          {html_options values=$list_ref_supannEtuTypeDiplome_SEI output=$list_ref_supannEtuTypeDiplome_SEI selected=$ref_supannEtuTypeDiplome_SEI}
        </select>
        <select name="supannEtuTypeDiplome_SEI" id="supannEtuTypeDiplome_SEI"  >
          {html_options options=$list_supannEtuTypeDiplome_SEI selected=$supannEtuTypeDiplome_SEI}
        </select>
      </td>


    </tr>
    <tr><!-- saisie   RANG 2 supannEtuRegimeInscription supannEtuRegimeInscription  -->
      <td>
        <label for="ref_supannEtuRegimeInscription_SEI">{t}supannEtuRegimeInscription{/t}</label>
      </td>
      <!-- [regimeinsc=<supannEtuRegimeInscription>] -->
      <td>
        <select name="ref_supannEtuRegimeInscription_SEI" id="ref_supannEtuRegimeInscription_SEI"  onChange="document.mainform.submit();" >
          {html_options values=$list_ref_supannEtuRegimeInscription_SEI output=$list_ref_supannEtuRegimeInscription_SEI selected=$ref_supannEtuRegimeInscription_SEI}
        </select>
        <select name="supannEtuRegimeInscription_SEI" id="supannEtuRegimeInscription_SEI"  >
          {html_options options=$list_supannEtuRegimeInscription_SEI selected=$supannEtuRegimeInscription_SEI}
        </select>
      </td>
      <td></td>
      <td>
        <label for="ref_supannEtuSecteurDisciplinaire_SEI">{t}supannEtuSecteurDisciplinaire{/t}</label>
      </td>
      <td>
        <!-- [*=<supannEtuSecteurDisciplinaire>]    -->
        <select name="ref_supannEtuSecteurDisciplinaire_SEI" id="ref_supannEtuSecteurDisciplinaire_SEI"  onChange="document.mainform.submit();">
          {html_options values=$list_ref_supannEtuSecteurDisciplinaire_SEI output=$list_ref_supannEtuSecteurDisciplinaire_SEI selected=$ref_supannEtuSecteurDisciplinaire_SEI}
        </select>
        <select name="supannEtuSecteurDisciplinaire_SEI" id="supannEtuSecteurDisciplinaire_SEI"  >
          {html_options options=$list_supannEtuSecteurDisciplinaire_SEI selected=$supannEtuSecteurDisciplinaire_SEI}
        </select>
      </td>
    </tr>
    <tr><!-- saisie   RANG 3 supannEtuAnneeInscription   supannEtuTypeDiplome -->
      <td>
        <label for="supannEtuAnneeInscription_SEI">{t}supannEtuAnneeInscription{/t}</label>
      </td>
      <td>
        <!-- [anneeinsc=<supannEtuAnneeInscription>] -->
        <input id="supannEtuAnneeInscription_SEI" name="supannEtuAnneeInscription_SEI" size="10" maxlength="50" type="text">
      </td>
      <td></td>
      <td>
        <label for="ref_supannEtuDiplome_SEI">{t}supannEtuDiplome{/t}</label>
      </td>
      <td>
        <!-- [diplome=<supannEtuDiplome>] -->
        <select name="ref_supannEtuDiplome_SEI" id="ref_supannEtuDiplome_SEI"  onChange="document.mainform.submit();">
          {html_options values=$list_ref_supannEtuDiplome_SEI output=$list_ref_supannEtuDiplome_SEI selected=$ref_supannEtuDiplome_SEI}
        </select>
        <select name="supannEtuDiplome_SEI" id="supannEtuDiplome_SEI">
          {html_options options=$list_supannEtuDiplome_SEI selected=$supannEtuDiplome_SEI}
        </select>
      </td>
    </tr>
    <tr><!-- saisie   RANG 4 supannEtuCursusAnnee  supannEntiteAffectation -->
      <td>
        <label for="ref_supannEtuCursusAnnee_SEI">{t}supannEtuCursusAnnee{/t}</label>
      </td>
      <td>
        <!-- [cursusann=<supannEtuCursusAnnee>] -->
        <select id="ref_supannEtuCursusAnnee_SEI" name="ref_supannEtuCursusAnnee_SEI" onChange="document.mainform.submit();">
          {html_options options=$list_supannEtuCursusAnnee_SEI selected=$ref_supannEtuCursusAnnee_SEI}
        </select>
        <select id="supannEtuCursusAnnee_SEI" name="supannEtuCursusAnnee_SEI">
          {html_options options=$years_supannEtuCursusAnnee selected=$supannEtuCursusAnnee_SEI}
        </select>
      </td>
      <td></td>
      <td>
        <label for="supannEntiteAffectation_SEI">{t}supannEntiteAffectation{/t}</label>
      </td>
      <td>
      <!-- [affect=<supannEntiteAffectation>] -->
        <select name="supannEntiteAffectation_SEI" id="supannEntiteAffectation_SEI"  >
          {html_options options=$list_supannEntiteAffectation_SEI selected=$supannEntiteAffectation_SEI}
        </select>
      </td>
    </tr>
    <tr><!-- saisie   RANG 5 supannEtuEtape  supannEtuElementPedagogique-->
      <td>
        <label for="ref_supannEtuEtape_SEI">{t}supannEtuEtape{/t}</label>
      </td>
      <td>
        <!-- [etape=<supannEtuEtape>] -->
        <select name="ref_supannEtuEtape_SEI" id="ref_supannEtuEtape_SEI"  onChange="document.mainform.submit();">
          {html_options values=$list_ref_supannEtuEtape_SEI output=$list_ref_supannEtuEtape_SEI selected=$ref_supannEtuEtape_SEI}
        </select>
        <select name="supannEtuEtape_SEI" id="supannEtuEtape_SEI"  >
          {html_options options=$list_supannEtuEtape_SEI selected=$supannEtuEtape_SEI}
        </select>
      </td>
      <td></td>
      <td>
        <label for="ref_etuelementpedagogique_SEI">{t}supannEtuElementPedagogique{/t}</label>
      </td>
      <td>
        <!-- [eltpedago=<supannEtuElementPedagogique>]  -->
        <select name="ref_supannEtuElementPedagogique_SEI" id="ref_etuelementpedagogique_SEI"  onChange="document.mainform.submit();">
          {html_options values=$list_ref_supannEtuElementPedagogique_SEI output=$list_ref_supannEtuElementPedagogique_SEI selected=$ref_supannEtuElementPedagogique_SEI}
        </select>
        <select name="supannEtuElementPedagogique_SEI" id="supannEtuElementPedagogique_SEI"  >
          {html_options options=$list_supannEtuElementPedagogique_SEI selected=$supannEtuElementPedagogique_SEI}
        </select>
      </td>
    </tr>
    <tr>
      <td colspan="2">
        <input name="addsupannEtuInscription" value="{msgPool type=addButton}" id="addsupannEtuInscription" type="submit">
        <input name="delsupannEtuInscription" value="{msgPool type=delButton}" id="delsupannEtuInscription" type="submit">
      </td>
    </tr>

</tbody>
</table>
{/if}

<input name="supannAccountTab" value="supannAccountTab" type="hidden">
