using Microsoft.AspNetCore.Builder;
using Microsoft.AspNetCore.Server.Kestrel.Core;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using klog_listener.Extensions;
using klog_listener.Helpers;

namespace klog_listener
{
    public class Startup
    {
        private readonly IConfiguration _configuration;

        public Startup(IConfiguration configuration)
        {
            _configuration = configuration;
        }

        public void ConfigureServices(IServiceCollection services)
        {
            services.Configure<KestrelServerOptions>(_configuration.GetSection("Kestrel"));
            services.AddRouting(options => 
            {
                options.LowercaseUrls = true;
            });
            services.AddControllers(options =>
            {
                options.InputFormatters.Insert(options.InputFormatters.Count, new TextPlainInputFormatter());
            });
            services.AddKeyLogStore(_configuration);
        }

        public void Configure(IApplicationBuilder app)
        {
            app.UseRouting();
            app.UseEndpoints(endpoints =>
            {
                endpoints.MapControllers();
            });
            app.UseKeyLogStore();
        }
    }
}
