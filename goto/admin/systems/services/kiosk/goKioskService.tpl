<h2><img alt="" class="center" src="plugins/goto/images/kiosk.png" align="middle">&nbsp;<LABEL for="gotoKioskProfile">{t}Kiosk profile management{/t}</ LABEL></h2>

{if $baseDir == ""}

<b>{msgPool type=invalidConfigurationAttribute param=KIOSKPATH}</b>

<p class='seperator'>&nbsp;</p>
<div style="width:100%; text-align:right;padding-top:10px;padding-bottom:3px;">
    <input type='submit' name='CancelService' value='{msgPool type=cancelButton}'>
</div>

{else}

    <input type="hidden" name="dialogissubmitted" value="1">

{t}Server path{/t}&nbsp;<input type='text' name="server_path" style="width:300px;" value="{$server_path}">
<br>
<br>
{render acl=$ThisACL}
{$divlist}
{/render}
{render acl=$ThisACL}
<input type="file" size=50 name="newProfile" value="{t}Browse{/t}">
{/render}
{render acl=$ThisACL}
<input type="submit" name="profileAdd" value="{msgPool type=addButton}">
{/render}

<p class='seperator'>&nbsp;</p>
<div style="width:100%; text-align:right;padding-top:10px;padding-bottom:3px;">
    <input type='submit' name='SaveService' value='{msgPool type=saveButton}'>
    &nbsp;
    <input type='submit' name='CancelService' value='{msgPool type=cancelButton}'>
</div>
<input type="hidden" name="goKioskPosted" value="1">

<script language="JavaScript" type="text/javascript">
  <!-- // First input field on page
    focus_field('gotoKioskProfile');
  -->
</script>
{/if}
