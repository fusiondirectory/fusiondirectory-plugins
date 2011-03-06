<table style="width:100%;">
	{if $enableReleaseManagement}
	<tr>
		<td>
			{t}Release{/t}
			<select name="FAIrelease" onChange="document.mainform.submit();">
			{foreach from=$releases item=item key=key}
				<option value="{$key}" {if $key == $FAIrelease} selected {/if}>{$item.name} </option>
			{/foreach}
			</select>
			<input type='image' src='images/lists/copy.png' title='{t}Copy menu{/t}' class='center' name='menu_copy'>
			{if $copied}
				<input type='image' src='images/lists/paste.png' title='{t}Paste menu from{/t}&nbsp;{$copy_source}' class='center' name='menu_paste'>
			{else}
				<input type='image' src='images/lists/paste-grey.png' class='center'>
			{/if}
			<input type='image' src='images/lists/delete.png' title='{t}Delete menu{/t}' class='center' name='menu_delete'>
		</td>
	</tr>
	{/if}
	<tr>
		<td style="width:50%; vertical-align:top;">
		<div style="height:290px; overflow:auto; 
					border-top: solid 2px #999999;
					border-left: solid 2px #999999;
					padding:5px; 
					">
			
<table style='width:100%' cellpadding=0 cellspacing=0>
{foreach from=$entries item=item key=key}
	{if $item.TYPE == "OPEN"}
		<tr>
			<td colspan=3 style="background-color: #DDDDDD;height:1px"></td>
		</tr>
		<tr>
			<td style='padding-left:20px;' colspan=3>
				<table style='width:100%;' cellpadding=0 cellspacing=0>

	{elseif $item.TYPE == "CLOSE"}
				</table>
			</td>
		</tr>
		<tr>
			<td colspan=3 style="background-color: #DDDDDD;height:1px"></td>
		</tr>
	{elseif $item.TYPE == "RELEASE"}
		{if $i++ % 2 == 0}
			<tr class="rowxp0">
		{else}
			<tr class="rowxp1">
		{/if}
			<td style='width:20px; padding-top:5px;padding-bottom:5px;background-color: transparent;'>
				<img src='plugins/fai/images/fai_small.png' alt='{t}Release{/t}'>
			</td>
			<td style='background-color: transparent;'>
				{$item.NAME}
			</td>
			<td style='width:100px;text-align:right;background-color: transparent;'>
			</td>
		</tr>
	{elseif $item.TYPE == "FOLDER"}
		{if $i++ %2 == 0}
			<tr class="rowxp0">
		{else}
			<tr class="rowxp1">
		{/if}
			<td width="22" style='width:22px; padding-top:3px;padding-bottom:3px; overflow:hidden;'>
				{if $item.ICON != ""}
					<div style="height:20px;width:20px; overflow:hidden;">
					<img class="center" src='?plug={$plug_id}&amp;send={$item.UNIQID}' alt='{t}Folder{/t}'>
					</div>
				{else}
					<div style="height:20px;width:20px; overflow:hidden">
					<img class="center" src='images/lists/folder.png' alt='{t}Folder{/t}'>
					</div>
				{/if}
			</td>
			<td style='background-color: transparent;'>
				<b>{$item.NAME}&nbsp; </b> 
			</td>
			<td style='width:100px;text-align:right; background-color: transparent;'>
				<input title="{t}Move up{/t}" 	class="center" type='image' 
					name='up_{$item.UNIQID}' src='images/move_object_up.png'>
				<input title="{t}Move down{/t}" class="center" type='image' 
					name='down_{$item.UNIQID}' src='images/move_object_down.png'>
				<input title="{t}Remove{/t}" 	class="center" type='image' 
					name='del_{$item.UNIQID}' src='images/lists/trash.png'>
				<input title="{t}Edit{/t}" 	 	class="center" type='image' 
					name='app_entry_edit{$item.UNIQID}' src='images/lists/edit.png'>
			</td>
		</tr>
	{elseif $item.TYPE == "SEPERATOR"}

		{if $i++ % 2 == 0}
			<tr class="rowxp0">
		{else}
			<tr class="rowxp1">
		{/if}
			<td style='background-color: transparent;width:22px; padding-top:5px;padding-bottom:5px;' colspan="2">
				<div style="height:3px; width:100%; background-color:#BBBBBB;"></div>
			</td>
            <td style='width:100px;text-align:right; background-color: transparent;'>
                <input title="{t}Move up{/t}"   class="center" type='image'
                    name='up_{$item.UNIQID}' src='images/move_object_up.png'>
                <input title="{t}Move down{/t}" class="center" type='image'
                    name='down_{$item.UNIQID}' src='images/move_object_down.png'>
                <input title="{t}Remove{/t}"    class="center" type='image'
                    name='del_{$item.UNIQID}' src='images/lists/trash.png'>
				<img src="images/empty.png" style="width:16px;" alt=" ">
            </td>
		</tr>
	{elseif $item.TYPE == "ENTRY"}

		{if $i++ % 2 == 0}
			<tr class="rowxp0">
		{else}
			<tr class="rowxp1">
		{/if}
			<td style='background-color: transparent;width:22px; padding-top:5px;padding-bottom:5px;'>
				<div style="width:20px; overflow:hidden; text-align:center;">
					<img src='plugins/goto/images/select_application.png' alt='{t}Entry{/t}' class="center">
				</div>
			</td>
			<td style="background-color: transparent;">
				{$item.NAME} {$item.INFO}
			</td>
			<td style='width:100px;text-align:right;background-color: transparent;'>
				<input title="{t}Move up{/t}" 	class="center" type='image' 
					name='up_{$item.UNIQID}' src='images/move_object_up.png'>
				<input title="{t}Move down{/t}" class="center" type='image' 
					name='down_{$item.UNIQID}' src='images/move_object_down.png'>
				<input title="{t}Remove{/t}" 	class="center" type='image' 
					name='del_{$item.UNIQID}' src='images/lists/trash.png'>
				<input title="{t}Edit{/t}" 	 	class="center" type='image' 
					name='app_entry_edit{$item.UNIQID}' src='images/lists/edit.png'>
			</td>
		</tr>
	{/if}
{/foreach}
</table>
		</div>
			<input type="text" name="menu_folder_name" value="">
			{t}add to{/t}
			<select name="menu_folder">
			{foreach from=$folders item=item key=key}
				<option value="{$key}">{$item}</option>
			{/foreach}
			</select>
			<input type="submit" name="add_menu_to_folder" value="{msgPool type=addButton}" title="{t}Add selected applications to this folder.{/t}">
			<input type="submit" name="add_seperator" 	   value="{t}Separator{/t}" title="{t}Add a separator to this folder.{/t}">
		</td>
		<td style="vertical-align:top">
			{$app_list}	
			<select name="folder">
			{foreach from=$folders item=item key=key}
				<option value="{$key}">{$item}</option>
			{/foreach}
			</select>
			<input type="submit" name="add_to_folder" value="{msgPool type=addButton}" title="{t}Add selected applications to this folder.{/t}">
		</td>
	</tr>
</table>
	
