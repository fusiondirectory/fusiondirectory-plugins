<table summary="{t}Phone numbers{/t}" style="width:100%">
 <tr>
  <td style="vertical-align:top; border-right:1px solid #A0A0A0; width:50%">
   <h2>
    <img class="center" alt="" align="middle" src="plugins/gofon/images/banana.png" />
    &nbsp;{t}Phone numbers{/t}
   </h2>

{if $multiple_support}

   <select style="width:100%;" name="dummy1" size=7 >
    {html_options options=$phoneNumbers}
    <option disabled>&nbsp;</option>
   </select>

{else}

{render acl=$telephoneNumberACL}
   <select style="width:100%;" name="phonenumber_list[]" size=7 multiple>
    {html_options options=$phoneNumbers}
    <option disabled>&nbsp;</option>
   </select>
{/render}
   <br>
{render acl=$telephoneNumberACL}
   <input type='text' name="phonenumber" size=20 align=middle maxlength=60 value="">
{/render}
{render acl=$telephoneNumberACL}
   <input type="submit" value="{msgPool type=addButton}" name="add_phonenumber">&nbsp;
{/render}
{render acl=$telephoneNumberACL}
      <input type="submit" value="{msgPool type=delButton}" name="delete_phonenumber">
{/render}

{/if}
  </td>
  <td style="vertical-align:top; width:50%;">
   <table summary="" style="width:100%" border=0>
 <tr>

  <td style="vertical-align:top;">
   <h2><img class="center" alt="" align="middle" src="plugins/gofon/images/hardware.png" />&nbsp;{t}Telephone hardware{/t}</h2>

      <table summary="{t}Telephone{/t}" border=0>
	   {if !$multiple_support}
       <tr>
        <td>
			{t}Telephone{/t}
		</td>
        <td>
		{render acl=$goFonHardwareACL checkbox=$multiple_support checked=$use_goFonHardware}
			{$hardware_list}
		{/render}
		</td>
       </tr>
	   {/if}
       <tr>
        <td>{t}Home server{/t}{$must}
        </td>
        <td>
{render acl=$goFonHomeServerACL checkbox=$multiple_support checked=$use_goFonHomeServer}
         <select name='goFonHomeServer' title='{t}Select the accounts home server{/t}'>
          {html_options options=$goFonHomeServers selected=$goFonHomeServer}
         </select>
{/render}
        </td>
      </tr>
    <tr>
     <td>{t}Context{/t}
     </td>
     <td>
{render acl=$goFonContextACL checkbox=$multiple_support checked=$use_goFonContext}
      <select name='context' title='{t}Select the accounts context{/t}'>
        {html_options values=$sip_contexts output=$sip_contexts selected=$context}
      </select>
{/render}
     </td>
    </tr>
    <tr>
     <td>{t}Voicemail context{/t}
     </td>
     <td>
{render acl=$goFonVoiceMailContextACL checkbox=$multiple_support checked=$use_goFonVoiceMailContext}
      <select name='voice_context' title='{t}Select the accounts context{/t}'>
        {html_options values=$voicemail_contexts output=$voicemail_contexts selected=$voice_context}
      </select>
{/render}
     </td>
    </tr>

      <tr>
       <td>
        <label for="goFonVoicemailPIN">{t}Voicemail PIN{/t}{$must}</label>
       </td>
       <td>
{render acl=$goFonVoicemailPINACL checkbox=$multiple_support checked=$use_goFonVoicemailPIN}
        <input type="password" id="goFonVoicemailPIN" name="goFonVoicemailPIN" value="{$goFonVoicemailPIN}">
{/render}
      </td>
      </tr>
      <tr>
       <td>
        <label for="goFonPIN">{t}Phone PIN{/t}{$must}</label>
       </td>
       <td>
{render acl=$goFonPINACL checkbox=$multiple_support checked=$use_goFonPIN}
        <input type="password" id="goFonPIN" name="goFonPIN" value="{$goFonPIN}">
{/render}
      </td>
      </tr>
      </table>
     </td>
    </tr>
   </table>
  </td>

 </tr>
 <tr>
 <td style="vertical-align:top; border-top:1px solid #A0A0A0; width:50%" colspan="2">&nbsp;</td>
 </tr>
 <tr style="vertical-align:top; border-top:1px solid #A0A0A0;">
 <td colspan=2 style="vertical-align:top;width:50%">
  <h2>
   <img class="center" alt="" src="plugins/gofon/images/select_macro.png" align="middle" />
   &nbsp;{t}Phone macro{/t}
  </h2>
  <table summary="{t}Phone macro{/t}">
   <tr>
    <td colspan=2>
{render acl=$goFonMacroACL  checkbox=$multiple_support checked=$use_macro}
     <select id="macro" name="macro" onchange="document.mainform.submit()">
      {html_options options=$macros selected=$macro}
      <option disabled>&nbsp;</option>
     </select>
{/render}
     {if $javascript eq 'false'}
{render acl=$goFonMacroACL}
      <input type="submit" name="refresh" value="{t}Refresh{/t}">
{/render}
     {/if}
    </td>
   </tr>
  </table>
  <br>
  {$macrotab}
 </td>
 </tr>
</table>
<input type="hidden" name="phoneTab" value="phoneTab">


<script language="JavaScript" type="text/javascript">
  <!-- // First input field on page
	focus_field('phonenumber');
  -->
</script>
