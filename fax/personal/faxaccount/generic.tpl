<table style="width:100%; vertical-align:top; text-align:left;" cellpadding="0" border="0">

 <!-- Headline container -->
 <tr>
   <td style="width:50%; vertical-align:top;">
     <h2><img class="center" alt="" src="images/rightarrow.png" />&nbsp;{t}Generic{/t}</h2>

     <table>
       <tr>
         <td><label for="facsimileTelephoneNumber">{t}Fax{/t}</label>{$must}</td>
         <td>
{render acl=$facsimileTelephoneNumberACL}
           <input name="facsimileTelephoneNumber" id="facsimileTelephoneNumber" size=20 maxlength=65
    value="{$facsimileTelephoneNumber}" title="{t}Fax number for fax to trigger on{/t}">
{/render}
         </td>
       </tr>
       <tr>
         <td><label for="goFaxLanguage">{t}Language{/t}</label></td>
   <td>

{render acl=$goFaxLanguageACL checked=$use_goFaxLanguage}
           <select size="1" name="goFaxLanguage" id="goFaxLanguage"
    title="{t}Specify the fax communication language for fax to mail gateway{/t}">
      {html_options options=$languages selected=$goFaxLanguage}
           </select>
{/render}

         </td>
       </tr>
       <tr>
         <td><label for="goFaxFormat">{t}Delivery format{/t}</label></td>
         <td>

{render acl=$goFaxFormatACL checked=$use_goFaxFormat}
           <select id="goFaxFormat" size="1" name="goFaxFormat" title="{t}Specify delivery format for fax to mail gateway{/t}">
      {html_options values=$formats output=$formats selected=$goFaxFormat}
           </select>
{/render}
         </td>
       </tr>
     </table>

   </td>
   <td style="border-left:1px solid #A0A0A0">
    &nbsp;
   </td>
   <td style="vertical-align:top; width:100%">
     <h2><img class="center" alt="" src="plugins/fax/images/printer.png" />&nbsp;{t}Delivery methods{/t}</h2>

{render acl=$goFaxIsEnabledACL checked=$use_goFaxIsEnabled}
     <input type=checkbox name="goFaxIsEnabled" value="1" {$goFaxIsEnabled} class="center">
{/render}
     {t}Temporary disable fax usage{/t}<br>

     {if $has_mailaccount eq "false"}
{render acl=$faxtomailACL checked=$use_faxtomail}
     <input type=checkbox name="faxtomail" value="1" {$faxtomail} class="center">
{/render}
      <label for="mail">{t}Deliver fax as mail to{/t}</label>&nbsp;
{render acl=$faxtomailACL checked=$use_mail}
      <input type='text' name="mail" id="mail" size=25 maxlength=65 value="{$mail}" class="center">
{/render}
     {else}
{render acl=$faxtomailACL checked=$use_faxtomail}
     <input type=checkbox name="faxtomail" value="1" {$faxtomail} class="center">
{/render}
      {t}Deliver fax as mail{/t}
     {/if}
     <br>

{render acl=$faxtoprinterACL checked=$use_faxtoprinter}
     <input type=checkbox name="faxtoprinter" value="1" {$faxtoprinter} class="center">
{/render}
     {t}Deliver fax to printer{/t}&nbsp;
{render acl=$faxtoprinterACL checked=$use_goFaxPrinter}
     <select size="1" name="goFaxPrinter">
      {html_options options=$printers selected=$goFaxPrinter}
    <option disabled>&nbsp;</option>
     </select>
{/render}
   </td>
 </tr>
</table>

<p class="seperator">&nbsp;</p>

<table style="width:100%; vertical-align:top; text-align:left;" cellpadding=4 border=0>
  <tr>
    <td style="width:50%; border-right:1px solid #A0A0A0">
    <h2><img class="center" alt="" src="plugins/fax/images/iconMini.png">&nbsp;{t}Alternate fax numbers{/t}</h2>
{render acl=$facsimileAlternateTelephoneNumberACL}
    <select style="width:100%" name="alternate_list[]" size="10" multiple>
      {html_options values=$facsimileAlternateTelephoneNumber output=$facsimileAlternateTelephoneNumber}
      <option disabled>&nbsp;</option>
    </select>
{/render}
    <br>
{render acl=$facsimileAlternateTelephoneNumberACL}
    <input type='text' name="forward_address" size=20 maxlength=65 value="">
{/render}
{render acl=$facsimileAlternateTelephoneNumberACL}
    <input type=submit value="{msgPool type=addButton}" name="add_alternate">&nbsp;
{/render}
{render acl=$facsimileAlternateTelephoneNumberACL}
    <input type=submit value="{t}Add local{/t}" name="add_local_alternate" >&nbsp;
{/render}
{render acl=$facsimileAlternateTelephoneNumberACL}
    <input type=submit value="{msgPool type=delButton}" name="delete_alternate">
{/render}
   </td>
   <td style="vertical-align:top; width:50%">
      <h2><img class="center" alt="" src="images/false.png" />&nbsp;{t}Blocklists{/t}</h2>
      <table style="width:100%">
        <tr>
          <td>{t}Blocklists for incoming fax{/t}</td>
          <td>
{render acl=$goFaxRBlocklistACL checked=$use_edit_incoming}
            <input type=submit name="edit_incoming" value="{t}Edit{/t}">
{/render}
          </td>
        </tr>
        <tr>
          <td>{t}Blocklists for outgoing fax{/t}</td>
          <td>
{render acl=$goFaxSBlocklistACL checked=$use_edit_outgoing}
            <input type=submit name="edit_outgoing" value="{t}Edit{/t}">
{/render}
          </td>
        </tr>
      </table>
    </td>
  </tr>
</table>

<input type="hidden" name="faxTab" value="faxTab">

<!-- Place cursor -->
<script type="text/javascript">
  <!-- // First input field on page
  focus_field('facsimileTelephoneNumber');
  -->
</script>
