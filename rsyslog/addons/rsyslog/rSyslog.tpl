<div class="contentboxh">
 <p class="contentboxh"><img src="images/launch.png" align="right" alt="[F]">{t}Filter{/t}</p>
</div>
<div class="contentboxb" style="border-top:1px solid #B0B0B0; padding:0px;">
<table summary="" class="contentboxb" border=0>
  <tr>
    <td><img src="plugins/rsyslog/images/server.png" alt="" class="center">&nbsp;{t}Server{/t}:</td>
    <td width="20%">
      <select name='selected_server' onChange='document.mainform.submit();'>
        {foreach from=$servers item=item key=key}
          <option value='{$key}' {if $key == $selected_server} selected {/if}>{$item.cn}</option>
        {/foreach}
      </select>
    </td>
    <td><img src="plugins/rsyslog/images/workstation.png" alt="" class="center">&nbsp;{t}Host{/t}:</td>
    <td width="20%">
      <select name='selected_host' onChange='document.mainform.submit();'>
        {foreach from=$hosts item=item key=key}
          <option value='{$key}' {if $key == $selected_host} selected {/if}>{$item}</option>
        {/foreach}
      </select>
    </td>
    <td><img src="images/small_warning.png" alt="" class="center">&nbsp;{t}Severity{/t}:</td>
    <td>
      <select name='selected_priority' onChange='document.mainform.submit();'>
        {html_options values=$priorities options=$priorities selected=$selected_priority}
      </select>
    </td>
    <td width="">&nbsp;</td>
  </tr>
  <tr>
    <td><img src="plugins/rsyslog/images/clock.png" alt="" class="center">&nbsp;{t}From{/t}:</td>
    <td>
      <input type="text" id="startTime" name="startTime" class="date" style='width:100px' value="{$startTime}">
      <script type="text/javascript">
        {literal}
        var datepicker  = new DatePicker({ relative : 'startTime', language : '{/literal}{$lang}{literal}', keepFieldEmpty : true,
           enableCloseEffect : false, enableShowEffect : false });
        {/literal}
      </script>
    </td>
    <td>{t}till{/t}:</td>
    <td>
      <input type="text" id="stopTime" name="stopTime" class="date" style='width:100px' value="{$stopTime}">
      <script type="text/javascript">
        {literal}
        var datepicker  = new DatePicker({ relative : 'stopTime', language : '{/literal}{$lang}{literal}', keepFieldEmpty : true,
           enableCloseEffect : false, enableShowEffect : false });
        {/literal}
      </script>
    </td>
    <td><img src="images/lists/search.png" alt="" class="center">&nbsp;{t}Search{/t}:</td>
    <td>
      <input type='text' name='search_for' value='{$search_for}' style='width:250px;'>
    </td>
    <td><input type='submit' name='search' value="{t}Search{/t}"></td>
  </tr>
</table>
</div>

{if $result.status != 'ok'}
  <b>{t}Error{/t}: &nbsp;{$result.status}</b><br>
  {$result.error}<br>
{else}

  <br>
  <table style="width:100%; background-color:#B0B0B0; white-space:nowrap; overflow-x:scroll;" cellspacing="1" cellpadding="2">
    <thead>
      <tr style="background-color: #E8E8E8; height:26px;">
        <th>
          <a href='?plug={$plug_id}&amp;sort_value=DeviceReportedTime'>{t}Date{/t}
            {if $sort_value=="DeviceReportedTime"}{if $sort_type=="DESC"}{$downimg}{else}{$upimg}{/if}{/if}
          </a>
        </th>
        <th>
          <a href='?plug={$plug_id}&amp;sort_value=FromHost'>{t}Source{/t}
            {if $sort_value=="FromHost"}{if $sort_type=="DESC"}{$downimg}{else}{$upimg}{/if}{/if}
          </a>
        </th>
        <th>
          <a href='?plug={$plug_id}&amp;sort_value=SysLogTag'>{t}Header{/t}
            {if $sort_value=="SysLogTag"}{if $sort_type=="DESC"}{$downimg}{else}{$upimg}{/if}{/if}
          </a>
        </th>
        <th>
          <a href='?plug={$plug_id}&amp;sort_value=Facility'>{t}Facility{/t}
            {if $sort_value=="Facility"}{if $sort_type=="DESC"}{$downimg}{else}{$upimg}{/if}{/if}
          </a>
        </th>
        <th>
          <a href='?plug={$plug_id}&amp;sort_value=Priority'>{t}Severity{/t}
            {if $sort_value=="Priority"}{if $sort_type=="DESC"}{$downimg}{else}{$upimg}{/if}{/if}
          </a>
        </th>
        <th>
          <a href='?plug={$plug_id}&amp;sort_value=Message'>{t}Message{/t}
            {if $sort_value=="Message"}{if $sort_type=="DESC"}{$downimg}{else}{$upimg}{/if}{/if}
          </a>
        </th>
      </tr>
    </thead>
    <tbody>
      {foreach from=$result.entries item=item key=key}

      {if ($key%2)}
      <tr style='background-color: #ECECEC;'>
      {else}
      <tr style='background-color: #F5F5F5;'>
      {/if}
        <td title='{$item.devicereportedtime}' style="padding-left:10px; padding-right:10px">
          {$item.devicereportedtime}
        </td>
        <td title='{$item.fromhost}' style="padding-left:10px; padding-right:10px">
          {$item.fromhost}
        </td>
        <td title='{$item.syslogtag}' style="padding-left:10px; padding-right:10px">
          {$item.syslogtag}
        </td>
        <td title='{$item.facility}' style="padding-left:10px; padding-right:10px">
          {$item.facility}
        </td>
        <td title='{$item.priority}' style="padding-left:10px; padding-right:10px">
          {$item.priority}
        </td>
        <td title='{$item.message}' style="padding-left:10px; padding-right:10px">
          {$item.message}
        </td>
      </tr>
      {/foreach}
    </tbody>
  </table>
  {if !$result.count == 0}
  <p class="separator">&nbsp;</p>
  {/if}
  <div style='width:40%;float:left;'>{$matches}</div>
  <div style='width:80px;float:right;'>
    <select name='limit' onChange='document.mainform.submit();'>
      {html_options options=$limits selected=$limit}
    </select>
  </div>
  <div style='width:300px;float:left;'>{$page_sel}</div>
{/if}

