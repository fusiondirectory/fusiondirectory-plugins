<div id="{$sectionId}" class="plugin_section{$sectionClasses}">
  <span class="legend">
    {$section}
  </span>
  <div>
    {foreach from=$attributes item=infos}
    <ul class="inventory">
      {foreach from=$infos key=class item=info}
        <li class="{$class}">
          <a href="#" onclick="Effect.toggle('toggle_{$class}', 'blind', {literal}{ duration: 0.3 }{/literal}); return false;">
            {$class}
          </a>
          <div id="toggle_{$class}" style="overflow: visible; display: none;">
            <table class="listingTable">
              <thead>
                <tr>
                  {foreach from=$info.keys item=label}
                    <th>{$label}</th>
                  {/foreach}
                </tr>
              </thead>
              <tbody>
                {foreach from=$info.objects key=cn item=object}
                  <tr>
                    {foreach from=$info.keys item=key}
                      <td>
                        {if isset($object.$key)}
                          {$object.$key}
                        {else}
                          &nbsp;
                        {/if}
                      </td>
                    {/foreach}
                  </tr>
                {/foreach}
              </tbody>
            </table>
          </div>
      {/foreach}
    </ul>
    {/foreach}
  </div>
</div>

