<h1>tbOS</h1>

<p>Projeto experimental de SO em x86 com boot em dois estágios e kernel carregado a partir de Lua.</p>

<p>O primeiro estágio continua em assembly de 16 bits apenas para carregar o segundo estágio. O segundo estágio agora entra em modo protegido, chama código ANSI C freestanding e prepara a transição para um kernel descrito em <code>kernel.lua</code>.</p>

<h2>Arquitetura atual</h2>

<ul>
  <li><code>bootloader.asm</code>: stage1 BIOS que carrega o stage2 para a memória.</li>
  <li><code>stage2_entry.asm</code>: entrada mínima em assembly, GDT e troca para modo protegido.</li>
  <li><code>stage2.c</code>: núcleo inicial em C, saída em VGA e orquestração do runtime.</li>
  <li><code>lua_runtime.c</code>: integra o core do Lua 5.4 ao ambiente freestanding.</li>
  <li><code>tb_libc.c</code>, <code>tb_heap.c</code> e <code>tb_jump.asm</code>: camada de suporte mínima para o runtime.</li>
  <li><code>kernel.lua</code>: fonte do kernel de alto nível embarcada no stage2 e executada no boot.</li>
</ul>

<h2>Build</h2>

<pre><code>make run</code></pre>

<p>Estado atual: o core do Lua é embarcado diretamente no stage2 e executa <code>kernel.lua</code> no boot. O ambiente expõe primitivas mínimas de console via <code>print</code> e <code>tb.write</code>.</p>
