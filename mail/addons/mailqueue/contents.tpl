<div style="height: 5px;">&nbsp;</div>
<div class="contentboxh">
 <p class="contentboxh"><img src="images/launch.png" alt="[F]" align="right">Filter</p>
</div>
<div class="contentboxb">
  <div >
	<table width="100%" summary="">
		<tr>
			<td>
	<img class="center" alt="{t}Search{/t}" src="images/lists/search.png" align="middle" border="0">
	 &nbsp;{t}Search for{/t}
    <input type='text' name="search_for" size="12" maxlength="60" value="{$search_for}" title="{t}Please enter a search string here.{/t}" onchange="mainform.submit()">
 	 &nbsp;in 
	<select size="1" name="p_server" title="{t}Select a server{/t}" onchange="mainform.submit()">
	 {html_options values=$p_serverKeys output=$p_servers selected=$p_server}
	</select>
<!--
	{t}with status{/t} : 
	<select size="1" name="Stat" onchange="mainform.submit()">
	 {html_options values=$stats output=$r_stats selected=$stat}
	</select>
-->
	 &nbsp;{t}within the last{/t}&nbsp;
	<select size="1" name="p_time" onchange="mainform.submit()">
	 {html_options values=$p_timeKeys output=$p_times selected=$p_time}
	</select>
	 &nbsp;
	<input name="search" value="{t}Search{/t}" type="submit">
			</td>
			<td style="border-left:1px solid #A0A0A0; text-align:right;">&nbsp;
				{if $delAll_W}
				<input name="all_del"  src="images/lists/trash.png"		
					value="{t}Remove all messages{/t}" type="image" 
					title="{t}Remove all messages from selected servers queue{/t}">
				{/if}
				{if $holdAll_W}
				<input name="all_hold" src="plugins/mail/images/mailq_hold.png"
					value="{t}Hold all messages{/t}" type="image"
					title="{t}Hold all messages in selected servers queue{/t}">
				{/if}
				{if $unholdAll_W}
				<input name="all_unhold" src="plugins/mail/images/mailq_unhold.png"		
					value="{t}Release all messages{/t}" 	type="image"
					title="{t}Release all messages in selected servers queue{/t}">
				{/if}
				{if $requeueAll_W}
				<input name="all_requeue" src="images/lists/reload.png"		
					value="{t}Requeue all messages{/t}" type="image"
					title="{t}Requeue all messages in selected servers queue{/t}">
				{/if}
   			</td>
		</tr>
	</table>
	</div>
</div>
<br>

{if !$query_allowed}
	<b>{msgPool type=permView}</b>
{else}

{if $all_ok != true}
<b>{t}Search returned no results{/t}...</b>
{else}

<table style="border: 1px solid rgb(176, 176, 176); width: 100%; vertical-align: top; text-align: left;" summary=""
 border="0" cellpadding="2" cellspacing="1" rules="cols">
	<tr style="background-color: rgb(232, 232, 232); height: 26px; font-weight: bold;">
		<td style='width:20px'>
    		<input type='checkbox' id='select_all' name='select_all' title='"._("Select all")."'
               onClick="toggle_all_('^selected_.*$','select_all');"></td>
		<td><a href="{$plug}&amp;sort=MailID"		>{t}ID{/t}			{if $OrderBy == "MailID"}	{$SortType}{/if}</a></td>
		<td><a href="{$plug}&amp;sort=Server"		>{t}Server{/t}		{if $OrderBy == "Server"}	{$SortType}{/if}</a></td>
		<td><a href="{$plug}&amp;sort=Size"			>{t}Size{/t}		{if $OrderBy == "Size"}		{$SortType}{/if}</a></td>
		<td><a href="{$plug}&amp;sort=Arrival"		>{t}Arrival{/t}		{if $OrderBy == "Arrival"}	{$SortType}{/if}</a></td>
		<td><a href="{$plug}&amp;sort=Sender"		>{t}Sender{/t}		{if $OrderBy == "Sender"}	{$SortType}{/if}</a></td>
		<td><a href="{$plug}&amp;sort=Recipient"	>{t}Recipient{/t}	{if $OrderBy == "Recipient"}{$SortType}{/if}</a></td>
		<td><a href="{$plug}&amp;sort=Status"		>{t}Status{/t}		{if $OrderBy == "Status"}	{$SortType}{/if}</a></td>
		<td>&nbsp;</td>
	</tr>

{counter start=0 assign=i start=1}
{foreach from=$entries item=val key=key}
			
	{if ($i%2)==0}
		<tr style="height: 22px; background-color: rgb(236, 236, 236);">
	{else}
		<tr style="height: 22px; background-color: rgb(245, 245, 245);">
	{/if}
		<td><input id="selected_{$entries[$key].MailID}" type='checkbox' name='selected_{$entries[$key].MailID}_{$entries[$key].Server}' class='center'></td>
		<td >
			{if $entries[$key].Active == true}
				<img class="center" src="plugins/mail/images/mailq_active.png" border=0 alt="{t}Active{/t}">
			{/if}
			{$entries[$key].MailID}</td>
		<td>{$entries[$key].ServerName}</td>
		<td>{$entries[$key].Size}</td>
		<td>{$entries[$key].Arrival|date_format:"%d.%m.%Y %H:%M:%S"}</td>
		<td>{$entries[$key].Sender}</td>
		<td>{$entries[$key].Recipient}</td>
		<td >{$entries[$key].Status}</td>
		<td style="text-align:right">
			{if $del_W}
				<input type='image' name='del__{$entries[$key].MailID}__{$entries[$key].Server}' class="center" 
					src="images/lists/trash.png" alt="{t}delete{/t}" title="{t}Delete this message{/t}">
			{else}
				<img src='images/empty.png' alt=' '>
			{/if}
			{if $entries[$key].Hold == true}

				{if $unhold_W}
					<input type='image' name='unhold__{$entries[$key].MailID}__{$entries[$key].Server}' class="center"
						src="plugins/mail/images/mailq_unhold.png" alt="{t}unhold{/t}" title="{t}Release message{/t}">
				{else}
					<img src='images/empty.png' alt=' '>
				{/if}
			{else}
				{if $hold_W}
					<input type='image' name='hold__{$entries[$key].MailID}__{$entries[$key].Server}' class="center"
						src="plugins/mail/images/mailq_hold.png" alt="{t}hold{/t}" title="{t}Hold message{/t}">
				{else}
					<img src='images/empty.png' alt=' '>
				{/if}
			{/if}
			{if $requeue_W}
				<input type='image' name='requeue__{$entries[$key].MailID}__{$entries[$key].Server}' class="center"
					src="images/lists/reload.png" alt="{t}requeue{/t}" title="{t}Requeue this message{/t}">
			{else}
				<img src='images/empty.png' alt=' '>
			{/if}
			{if $header_W}
				<input type='image' name='header__{$entries[$key].MailID}__{$entries[$key].Server}' class="center"
					src="plugins/mail/images/mailq_header.png" alt="{t}header{/t}" 
					title="{t}Display header of this message{/t}">
			{else}
				<img src='images/empty.png' alt=' '>
			{/if}
		</td>
	</tr>
	{counter}
{/foreach}
</table>

 <table summary="" style="width:100%; vertical-align:top; text-align:center;" cellpadding=4 cellspacing=0 border=0>
  <tr>
   <td>{$range_selector}</td>
  </tr>
 </table>
<p class="plugbottom">&nbsp;</p>

{/if}
{/if}


<script language="JavaScript" type="text/javascript">
  <!-- // First input field on page
	focus_field('search_for');
  -->
</script>
