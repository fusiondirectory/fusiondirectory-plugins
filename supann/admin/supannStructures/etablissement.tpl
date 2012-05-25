<table summary="" style="width:100%; vertical-align:top; text-align:left;" cellpadding=4>
 <tr>
   <td style="vertical-align:top; width:50%">
     <h2><img class="center" alt="" align="middle" src="images/rightarrow.png"> {t}Properties{/t}</h2>

     <table summary="">
{if $root_available}
      <tr>
        <td><label for="set_root">{t}Set this etablissement as root one{/t}{$must}</label></td>
        <td>
{render acl=$oACL}
          <input type='checkbox' id="set_root" name="set_root" />
{/render}
        </td>
      </tr>
{elseif $is_root}
      <tr>
        <td colspan="2"><h3>{t}This etablissement is the root one{/t}</h3></td>
      </tr>
{/if}
      <tr>
       <td><label for="o">{t}Name of etablissement{/t}{$must}</label></td>
       <td>
{render acl=$oACL}
          <input type='text' id="o" name="o" size=25 maxlength=60 value="{$o}"
          {if $is_root}
          disabled="disabled"
          {/if}
          />
{/render}
       </td>
      </tr>
      <tr>
       <td><label for="description">{t}Description{/t}</label></td>
       <td>
{render acl=$descriptionACL}
        <textarea id='description' name='description' style='width:220px'>{$description}</textarea>
{/render}
       </td>
      </tr>
      <tr>
       <td><label for="eduOrgHomePageURI">{t}eduOrgHomePageURI{/t}</label></td>
       <td>
{render acl=$eduOrgHomePageURIACL}
        <input type='text' id="eduOrgHomePageURI" name="eduOrgHomePageURI" size=25 maxlength=60 value="{$eduOrgHomePageURI}">
{/render}
       </td>
      </tr>
      <tr>
       <td><label for="eduOrgLegalName">{t}eduOrgLegalName{/t}{$must}</label></td>
       <td>
{render acl=$eduOrgLegalNameACL}
        <input type='text' id="eduOrgLegalName" name="eduOrgLegalName" size=25 maxlength=60 value="{$eduOrgLegalName}">
{/render}
       </td>
      </tr>
      <tr>
       <td><label for="eduOrgWhitePagesURI">{t}eduOrgWhitePagesURI{/t}</label></td>
       <td>
{render acl=$eduOrgWhitePagesURIACL}
        <input type='text' id="eduOrgWhitePagesURI" name="eduOrgWhitePagesURI" size=25 maxlength=60 value="{$eduOrgWhitePagesURI}">
{/render}
       </td>
      </tr>
      <tr>
       <td><label for="eduOrgSuperiorURI">{t}eduOrgSuperiorURI{/t}</label></td>
       <td>
{render acl=$eduOrgSuperiorURIACL}
        <input type='text' id="eduOrgSuperiorURI" name="eduOrgSuperiorURI" size=25 maxlength=60 value="{$eduOrgSuperiorURI}">
{/render}
       </td>
      </tr>
      <tr>
       <td><label for="supannEtablissement">{t}supannEtablissement{/t}{$must}</label></td>
       <td>
{render acl=$supannEtablissementACL}
        <input type='text' id="supannEtablissement" name="supannEtablissement" size=25 maxlength=60 value="{$supannEtablissement}">
{/render}
       </td>
      </tr>
      <tr>
       <td><label for="supannCodeEntite">{t}supannCodeEntite{/t}{$must}</label></td>
       <td>
{render acl=$supannCodeEntiteACL}
        <input type='text' id="supannCodeEntite" name="supannCodeEntite" size=25 maxlength=60 value="{$supannCodeEntite}">
{/render}
       </td>
      </tr>
     </table>

   </td>
   <td style="border-left:1px solid #A0A0A0">
    &nbsp;
   </td>
   <td>
     <h2><img class="center" alt="" align="middle" src="plugins/departments/images/department.png"> {t}Location{/t}</h2>

     <table summary="" style="width:100%">
      <tr>
       <td><label for="l">{t}Location{/t}</label></td>
       <td>
{render acl=$lACL}
	<input type='text' id="l" name="l" size=25 maxlength=60 value="{$l}" title="{t}Location of this subtree{/t}">
{/render}
       </td>
      </tr>
      <tr>
       <td style="vertical-align:top;"><label for="postalAddress">{t}Address{/t}</label></td>
       <td>
{render acl=$postalAddressACL}
	<textarea id="postalAddress" name="postalAddress" style="width:100%" rows=3 cols=22 title="{t}Postal address of this subtree{/t}">{$postalAddress}</textarea>
{/render}
      </tr>
      <tr>
       <td><label for="telephoneNumber">{t}Phone{/t}</label></td>
       <td>
{render acl=$telephoneNumberACL}
	<input type='text' id="telephoneNumber" name="telephoneNumber" size=25 maxlength=60 value="{$telephoneNumber}" title="{t}Base telephone number of this subtree{/t}">
{/render}
       </td>
      </tr>
      <tr>
       <td><label for="facsimileTelephoneNumber">{t}Fax{/t}</label></td>
       <td>
{render acl=$facsimileTelephoneNumberACL}
	<input type='text' id="facsimileTelephoneNumber" name="facsimileTelephoneNumber" size=25 maxlength=60 value="{$facsimileTelephoneNumber}" title="{t}Base facsimile telephone number of this subtree{/t}">
{/render}
       </td>
      </tr>
     </table>

   </td>
 </tr>
</table>

<p class='seperator'>&nbsp;</p>

<!-- Place cursor -->
<input type='hidden' name='etablissement_posted' value='1'>
<script language="JavaScript" type="text/javascript">
  <!-- // First input field on page
	focus_field('o');
  -->
</script>
