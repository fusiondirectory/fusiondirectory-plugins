<div id="{$sectionId}" class="plugin_section{$sectionClasses}">
  <span class="legend">
    {$section}
  </span>
  <div>
    {if is_array($attributes.dnsRecords)}
      <ul>
        {foreach from=$attributes.dnsRecords key=key item=node}
          {if isset($node.img)}
            <li style="list-style-image:url('{$node.img}');">
          {else}
            <li>
          {/if}
            <a href="#" onclick="Effect.toggle('toggle_{$key}', 'blind', {literal}{ duration: 0.3 }{/literal}); return false;">
              {$node.name}
            </a>
            <div id="toggle_{$key}" style="overflow: visible;">
              <table class="listingTable">
                <tbody>
                  {foreach from=$node.records item=record}
                    <tr>
                      <td>{$record.0|escape:'html':'UTF-8'}</td>
                      <td>{$record.1|escape:'html':'UTF-8'}</td>
                      <td>{$record.2|escape:'html':'UTF-8'}</td>
                    </tr>
                  {/foreach}
                </tbody>
              </table>
            </div>
          </li>
        {/foreach}
      </ul>
    {else}
      <img alt="{t}Insufficient rights{/t}" src="geticon.php?context=status&icon=dialog-error&size=16" class="center"/>&nbsp;
      <b>You have insufficient rights for using this tab</b>
    {/if}
  </div>
</div>

