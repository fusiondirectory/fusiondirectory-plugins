{if !$init_successfull}
<br>
<b>{msgPool type=siError}</b><br>
{t}Check if the GOsa support daemon (gosa-si) is running.{/t}&nbsp;
<input type='submit' name='retry_init' value="{t}Retry{/t}">
<br>
<br>
{else}

<h2>{t}License{/t}</h2>

<table style='width:100%'>
  <tr>
    <td style='width:50%; border-right: solid 1px #AAA; padding: 5px; vertical-align:top;'>
        
        <table>
          <tr>
            <td>
              {t}Name{/t}{$must}
            </td>
            <td>
              {if $initially_was_account}
                <input type='text' name='dummy12' disabled value='{$cn}'>
              {else}
{render acl=$licenseACL}
                <input type='text' name='cn' value='{$cn}'>
{/render}
              {/if}
            </td>
          </tr>
          <tr>
            <td>
              {t}Partner{/t}
            </td>
            <td>
{render acl=$licenseACL}
              <input type='text' name='partner' value='{$partner}'>
{/render}
            </td>
          </tr>
        </table>

    </td>
    <td style='padding: 5px; vertical-align:top;'>

        <table>
          <tr>
            <td>
              {t}Description{/t}
            </td>
            <td>
{render acl=$licenseACL}
              <input type='text' name='description' value='{$description}'>
{/render}
            </td>
          </tr>
        </table>

    </td>
  </tr>
  <tr>
    <td colspan="2"><p class='separator'>&nbsp;</p></td>
  </tr>
  <tr>
    <td style='border-right: solid 1px #AAA; padding: 5px; vertical-align:top;'>
    
        <table>
          <tr>
            <td>
              {t}Conclusion date{/t}
            </td>
            <td style='width:300px;'>
{render acl=$licenseACL}
              <input type="text" id="date1" name="conclusionDate" value="{$conclusionDate}" class="date" />
              {if $conclusionDateWriteable}
              {literal}
              <script type="text/javascript">
                  /*<[CDATA[*/
                  var datepicker    = new DatePicker({
                  keepFieldEmpty : true,
                  enableCloseEffect : false,
                  enableShowEffect : false,
                  relative    : 'date1',
                  language    : '{/literal}{$lang}{literal}'
                  });
                  /*]]>*/
              </script>
              {/literal}
              {/if}
{/render}
            </td>
          </tr>
          <tr>
            <td>
              {t}Expiration date{/t}
            </td>
            <td style='width:300px;'>
{render acl=$licenseACL}
              <input type="text" id="date2" name="expirationDate" value="{$expirationDate}" class="date" />
              {if $expirationDateWriteable}
              {literal}
              <script type="text/javascript">
                  /*<[CDATA[*/
                  var datepicker    = new DatePicker({
                  keepFieldEmpty : true,
                  enableCloseEffect : false,
                  enableShowEffect : false,
                  relative    : 'date2',
                  language    : '{/literal}{$lang}{literal}'
                  });
                  /*]]>*/
              </script>
              {/literal}
              {/if}
{/render}
            </td>
          </tr>
        </table>
 
    </td> 
    <td style='border-right: solid 1px #AAA; padding: 5px; vertical-align:bottom;'>
   
        <table>
          <tr>
            <td>
              {t}Notification date{/t}
            </td>
            <td style='width:300px;'>
{render acl=$licenseACL}
              <input type="text" id="date3" name="notificationDate" value="{$notificationDate}" class="date" >
              {if $notificationDateWriteable}
              {literal}
              <script type="text/javascript">
                  /*<[CDATA[*/
                  var datepicker    = new DatePicker({
                  keepFieldEmpty : true,
                  enableCloseEffect : false,
                  enableShowEffect : false,
                  relative    : 'date3',
                  language    : '{/literal}{$lang}{literal}'
                  });
                  /*]]>*/
              </script>
              {/literal}
              {/if}
{/render}
            </td>
          </tr>
        </table>
    </td> 
  </tr>
</table>

<p class='separator'>&nbsp;</p>

<h2>{t}License model{/t}</h2>

<table width="100%">
  <tr>
    <td style='width:50%;border-right: solid 1px #AAA; padding: 5px; vertical-align:top;'>
    
        <table>
          <tr>
            <td>
              {t}Model{/t}
            </td>
            <td>
              {if $initially_was_account}
                <select name='dummy223' disabled>
                  {html_options options=$licenseModels values=$licenseModels selected=$licenseModel}
                </select>
              {else}
                <select name='licenseModel' onChange='document.mainform.submit();'>
                  {html_options options=$licenseModels values=$licenseModels selected=$licenseModel}
                </select>
              {/if}
            </td>
          </tr>
        </table>
 
    </td> 
  </tr>
</table>

<p class='separator'>&nbsp;</p>

<table width="100%">
  <tr>
    <td style='width:50%;border-right: solid 1px #AAA; padding: 5px; vertical-align:top;'>
    
        <table>
          <tr>
            <td>
              {t}License key{/t}{$must}
            </td>
            <td>
{render acl=$licenseACL}
              <input type='text' name='licenseKey' value='{$licenseKey}'>
{/render}
            </td>
          </tr>
          {if $licenseModel == "VOLUME"}
          <tr>
            <td>
              {t}Maximum installations{/t}
            </td>
            <td>
{render acl=$licenseACL}
              <input type='text' name='maximumInstallations' value='{$maximumInstallations}'>
{/render}
            </td>
          </tr>
          {/if}
          {if $licenseModel == "OEM"}
          <tr>
            <td>
              {t}Reserved for{/t}
            </td>
            <td>
{render acl=$licenseACL}
              <select name='boundToHost'>
                <option value="">{t}none{/t}</option>
                {html_options options=$hosts selected=$boundToHost}
              </select>
{/render}
            </td>
          </tr>
          {/if}
        </table>
 
    </td> 
    <td style='border-right: solid 1px #AAA; padding: 5px; vertical-align:bottom;'>
   
        <table width="100%">
          <tr>
            <td colspan="2">
              <b>{t}Used by{/t}</b><br>
{render acl=$licenseACL}
              <select   disabled
                name='selectedUsedHosts[]' multiple size=4 style='width:100%;'>
                {html_options options=$usedByHost}
              </select><br>
{/render}
<!--
    
      Actually we can't modify the license usage, due to a lack of functions.
      We can only assign a licensePool to a host, but not a specific license.

              <select name='selectedHostToAdd'>
                {html_options options=$notUsedHosts}
              </select>
              <input type="submit" name="addLicenseUsage" value="{msgPool type='addButton'}">
              <input type="submit" name="removeLicenseUsage" value="{msgPool type='delButton'}">
-->
            </td>
          </tr>
        </table>
    </td> 
  </tr>
</table>
<input name='opsiLicensesPosted' value='1' type='hidden'>
{/if}
