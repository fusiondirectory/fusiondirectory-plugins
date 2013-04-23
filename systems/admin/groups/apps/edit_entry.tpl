
{if $type == "FOLDER"}
<h2>{$entry.NAME}</h2>

<table>
  <tr>
    <td>
      {t}Folder image{/t}
    </td>
    <td>
      {if $image_set}
        <img src="getbin.php?{$rand}" alt='{t}Could not load image.{/t}'>
      {else}
        <i>{t}None{/t}</i>
      {/if}
    </td>
  </tr>
  <tr>
    <td colspan=2>
      &nbsp;
    </td>
  </tr>
  <tr>
    <td>{t}Upload image{/t}
    </td>
    <td>
      <input type="FILE" name="folder_image">
      <input type='submit' name='folder_image_upload' value="{t}Upload{/t}">
    </td>
  </tr>
  <tr>
    <td>{t}Reset image{/t}</td>
    <td><input type='submit' name='edit_reset_image' value='{t}Reset{/t}'></td>
  </tr>
</table>
<br>
{/if}

{if $type == "ENTRY"}
<h2>{t}Application settings{/t}</h2>
<table>
  <tr>
    <td>{t}Name{/t}</td>
    <td>{$entry.NAME}</td>
  </tr>
  <tr>
    <td colspan="2">
      &nbsp;
    </td>
  </tr>
  <tr>
    <td colspan="2">
      <b>{t}Application options{/t}</b>
    </td>
  </tr>
{foreach from=$paras item=item key=key}
  <tr>
    <td>{$key}&nbsp;</td>
    <td><input style='width:200px;' type='text' name="parameter_{$key}" value="{$item}"></td>
  </tr>
{/foreach}
</table>
{/if}

<hr>
{render acl=$placeOnDesktopACL}
      <input type=checkbox name="placeOnDesktop" value="1" {$placeOnDesktop}>
{/render}
      {t}Place icon on members desktop{/t}
      <br>
{render acl=$placeInStartmenuACL}
      <input type=checkbox name="placeInStartmenu" value="1" {$placeInStartmenu}>
{/render}
      {t}Place entry in members startmenu{/t}
      <br>
{render acl=$placeOnKickerACL}
      <input type=checkbox name="placeOnKicker" value="1" {$placeOnKicker}>
{/render}
      {t}Place entry in members launch bar{/t}

<p class="seperator">
</p>
<div style="width:100%; text-align:right; padding:3px;">
  <input type="submit" name="app_entry_save" value="{msgPool type=saveButton}">
  &nbsp;
  <input type="submit" name="app_entry_cancel" value="{msgPool type=cancelButton}">
</div>
